pkgname=iptsd
pkgver=0.5
pkgrel=1
pkgdesc='Userspace daemon for Intel Precise Touch & Stylus'
arch=('x86_64')
url='https://github.com/linux-surface/iptsd'
license=('GPL')
depends=(
	'libfmt.so'
	'libinih'
	'libspdlog.so'
	'cairomm'
	'gtkmm3'
)
makedepends=(
	'meson'
	'gcc'
	'cmake'
	'microsoft-gsl'
	'systemd'
	'udev'
)

build() {
	cd $startdir

	arch-meson build --wrap-mode=default -Daccess_checks=disabled
	ninja -C build
}

check() {
	cd $startdir

	ninja -C build test
}

package() {
	cd $startdir

	DESTDIR="$pkgdir" ninja -C build install
}
