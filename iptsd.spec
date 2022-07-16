%global debug_package %{nil}

Name: iptsd
Version: 0.5.1
Release: 1%{?dist}
Summary: Userspace daemon for Intel Precise Touch & Stylus
License: GPLv2+

URL: https://github.com/linux-surface/iptsd
Source: {{{ git_dir_pack dir_name="iptsd" source_name="iptsd.tar.gz"  }}}

BuildRequires: meson
BuildRequires: gcc-g++

# Some of our dependencies can only be resolved with cmake
BuildRequires: cmake

# Daemon
BuildRequires: pkgconfig(fmt)
BuildRequires: pkgconfig(inih)
BuildRequires: cmake(Microsoft.GSL)
BuildRequires: pkgconfig(spdlog)

# Debug Tools
BuildRequires: cmake(CLI11)
BuildRequires: pkgconfig(cairomm-1.0)
BuildRequires: pkgconfig(gtkmm-3.0)

BuildRequires: pkgconfig(systemd)
BuildRequires: pkgconfig(udev)
BuildRequires: systemd-rpm-macros

%description
iptsd is a userspace daemon that processes touch events from the IPTS
kernel driver, and sends them back to the kernel using uinput devices.

%prep
{{{ git_dir_setup_macro dir_name="iptsd" }}}

%build
%meson
%meson_build

%install
%meson_install

%post
%systemd_post iptsd.service

%preun
%systemd_preun iptsd.service

%postun
%systemd_postun_with_restart iptsd.service

%check
%meson_test

%files
%license LICENSE
%doc README.md
%config(noreplace) %{_sysconfdir}/ipts.conf
%{_bindir}/iptsd
%{_bindir}/ipts-dump
%{_bindir}/ipts-proto-plot
%{_bindir}/ipts-proto-rt
%{_unitdir}/iptsd.service
%{_udevrulesdir}/50-ipts.rules
%{_datadir}/ipts/*
