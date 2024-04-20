// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_APPLICATION_HPP
#define IPTSD_CORE_GENERIC_APPLICATION_HPP

#include "config.hpp"
#include "device.hpp"
#include "dft.hpp"
#include "errors.hpp"

#include <common/casts.hpp>
#include <common/error.hpp>
#include <common/types.hpp>
#include <contacts/finder.hpp>
#include <ipts/parser.hpp>
#include <ipts/samples/button.hpp>
#include <ipts/samples/dft.hpp>
#include <ipts/samples/stylus.hpp>
#include <ipts/samples/touch.hpp>

#include <spdlog/spdlog.h>

#include <functional>
#include <vector>

namespace iptsd::core {

/*
 * The application class is the heart of iptsd.
 *
 * It handles all of the "common" tasks, like DFT stylus processing
 * and contact detection from capacitive heatmaps.
 *
 * The final data can then be processed further by extending this
 * class and overriding the appropriate methods.
 *
 * An application does not make any assumptions about the source
 * of the data it is receiving. For that reason, applications
 * need to be run by an application runner.
 */
class Application {
protected:
	/*
	 * The configuration for this application.
	 * This needs to be loaded by a platform specific loader
	 * and passed to the application during construction.
	 */
	Config m_config;

	/*
	 * Information about the device that produced the incoming data.
	 * This needs to be queried by the application runner
	 * and passed to the application during construction.
	 */
	DeviceInfo m_info;

	/*
	 * Parses incoming data and returns heatmap, stylus and DFT data.
	 */
	ipts::Parser m_parser {};

	/*
	 * Temporary storage for normalized heatmap data.
	 */
	Image<f64> m_heatmap {};

	/*
	 * The contact finder. This is where the magic happens.
	 *
	 * It accepts a normalized heatmap as the input, runs a gaussian-fitting based
	 * blob detection, contact tracking, and decides whether a contact is stable and valid.
	 */
	contacts::Finder<f64> m_finder;

	/*
	 * The list of contacts that the contact finder has found in the current frame.
	 */
	std::vector<contacts::Contact<f64>> m_contacts {};

	/*
	 * Newer devices use a DFT based stylus interface. Instead of sending already processed
	 * coordinates, these devices send antenna measurements that requires interpolating
	 * the position of the stylus manually.
	 */
	DftStylus m_dft;

public:
	Application(const Config &config, const DeviceInfo &info)
		: m_config {config},
		  m_info {info},
		  m_finder {config.contacts()},
		  m_dft {config, info}
	{
		if (m_config.width == 0 || m_config.height == 0)
			throw common::Error<Error::InvalidScreenSize> {};

		m_parser.on_touch = [&](const auto &data) { this->process_touch(data); };
		m_parser.on_stylus = [&](const auto &data) { this->process_stylus(data); };
		m_parser.on_dft = [&](const auto &data) { this->process_dft(data); };
		m_parser.on_button = [&](const auto &data) { this->process_button(data); };
	}

	virtual ~Application() = default;

	/*!
	 * Parse and process an IPTS data buffer.
	 *
	 * @param[in] data The buffer to process.
	 */
	void process(const gsl::span<u8> data)
	{
		this->on_data(data);
	}

	/*!
	 * For running application specific code after the runner has started.
	 */
	virtual void on_start() {};

	/*!
	 * For running application specific code after the runner has stopped.
	 */
	virtual void on_stop() {};

protected:
	/*!
	 * For replacing the parsing step of the data with application
	 * specific code that operates on the entire incoming data.
	 */
	virtual void on_data(const gsl::span<u8> data)
	{
		m_parser.parse(data);
	}

	/*!
	 * For running application specific code that further processes touch inputs.
	 */
	virtual void on_touch(const std::vector<contacts::Contact<f64>> & /* unused */) {};

	/*!
	 * For running application specific code that futher processes stylus inputs.
	 */
	virtual void on_stylus(const ipts::samples::Stylus & /* unused */) {};

	/*!
	 * For running application specific code that further processes button clicks.
	 */
	virtual void on_button(const ipts::samples::Button & /* unused */) {};

private:
	/*!
	 * Runs contact detection on an IPTS heatmap.
	 *
	 * IPTS usually sends data that goes from 255 (no contact) to 0 (contact).
	 * For contact detection we need data that goes from 0 (no contact) to 1 (contact).
	 *
	 * @param[in] data The data to process.
	 */
	void process_touch(const ipts::samples::Touch &data)
	{
		const Eigen::Index rows = casts::to_eigen(data.rows);
		const Eigen::Index cols = casts::to_eigen(data.columns);

		if (rows == 0 || cols == 0)
			return;

		// Make sure the heatmap buffer has the right size
		if (m_heatmap.rows() != rows || m_heatmap.cols() != cols)
			m_heatmap.conservativeResize(rows, cols);

		// Map the buffer to an Eigen container
		const Eigen::Map<const Image<u8>> mapped {data.heatmap.data(), rows, cols};

		const auto min = casts::to<f64>(data.min);
		const auto max = casts::to<f64>(data.max);

		// Normalize the heatmap to range [0, 1]
		const auto norm = (mapped.cast<f64>() - min) / (max - min);

		// IPTS sends inverted heatmaps
		m_heatmap = 1.0 - norm;

		// Search for contacts
		m_finder.find(m_heatmap, m_contacts);

		// Invert contact coordinates if neccessary
		for (contacts::Contact<f64> &contact : m_contacts) {
			if (m_config.invert_x)
				contact.mean.x() = 1.0 - contact.mean.x();

			if (m_config.invert_y)
				contact.mean.y() = 1.0 - contact.mean.y();

			if (m_config.invert_x != m_config.invert_y)
				contact.orientation = 1.0 - contact.orientation;
		}

		// Hand off the found contacts to the handler code.
		this->on_touch(m_contacts);
	}

	/*!
	 * Handles incoming IPTS stylus data.
	 *
	 * @param[in] data The data to process.
	 */
	void process_stylus(const ipts::samples::Stylus &data)
	{
		if (!m_info.is_touchscreen())
			return;

		ipts::samples::Stylus corrected = data;

		// Correct position based on tip-transmitter distance
		const Vector2<f64> off = this->calculate_offset(data.altitude, data.azimuth);
		corrected.x += off.x();
		corrected.y += off.y();

		// Hand off the stylus data to the handler code.
		this->on_stylus(corrected);
	}

	/*!
	 * Handles incoming DFT windows.
	 *
	 * DFT windows update the state of the DFT based stylus. The updated data is then
	 * processed exactly like older data, through @ref process_stylus.
	 *
	 * @param[in] data The DFT window to process.
	 */
	void process_dft(const ipts::samples::DftWindow &data)
	{
		if (!m_info.is_touchscreen())
			return;

		m_dft.input(data);
		this->process_stylus(m_dft.get_stylus());
	}

	/*!
	 * Handles incoming button clicks.
	 *
	 * A button sample describes the state of the left / right click button on
	 * IPTS touchpads.
	 *
	 * @param[in] data The data to process.
	 */
	void process_button(const ipts::samples::Button &data)
	{
		if (!m_info.is_touchpad())
			return;

		this->on_button(data);
	}

	/*!
	 * Calculates the tilt-based offset of the stylus position.
	 *
	 * Some styli have the transmitter a few millimeters above the tip of the pen.
	 * This means that the more you tilt the pen, the more the reported position will
	 * diverge from the position of the pen tip.
	 *
	 * If the distance between transmitter and pen tip is known, this offset can be
	 * calculated and added to the reported position.
	 *
	 * @param[in] altitude The altitude of the stylus.
	 * @param[in] azimuth The azimuth of the stylus.
	 * @return A Vector containing the offset on the X and Y axis.
	 */
	[[nodiscard]] Vector2<f64> calculate_offset(const f64 altitude, const f64 azimuth) const
	{
		if (altitude <= 0)
			return Vector2<f64>::Zero();

		if (m_config.stylus_tip_distance == 0)
			return Vector2<f64>::Zero();

		const f64 offset = std::sin(altitude) * m_config.stylus_tip_distance;

		const f64 ox = offset * -std::cos(azimuth);
		const f64 oy = offset * std::sin(azimuth);

		return Vector2<f64> {ox / m_config.width, oy / m_config.height};
	}
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_APPLICATION_HPP
