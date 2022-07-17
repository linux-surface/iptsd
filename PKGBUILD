pkgname=iptsd
pkgver=0.5.1
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

	arch-meson build --wrap-mode=default --force-fallback-for=hidrd_usage,hidrd_item,cli11
	meson compile -C build
}

check() {
	cd $startdir

	meson test -C build
}

package() {
	cd $startdir

	DESTDIR="$pkgdir" meson install -C build --skip-subprojects
}
