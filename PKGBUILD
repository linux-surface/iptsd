pkgname=iptsd
pkgver=3
pkgrel=1
pkgdesc='Userspace daemon for Intel Precise Touch & Stylus'
arch=('x86_64' 'aarch64')
url='https://github.com/linux-surface/iptsd'
license=('GPL')
depends=(
	'libgl'
	'libx11'
	'libxext'
	'libxcursor'
	'libxi'
	'libxfixes'
	'libxrandr'
	'libcairomm-1.0.so'
)
makedepends=(
	'meson'
	'gcc'
	'systemd'
	'udev'
)

build() {
	cd $startdir

	export CFLAGS="$(echo "$CFLAGS" | sed 's|-O2||g' | sed 's|-mtune=generic||g' | sed 's|-march=x86_64||g')"
	export CXXFLAGS="$(echo "$CXXFLAGS" | sed 's|-O2||g' | sed 's|-mtune=generic||g' | sed 's|-march=x86_64||g')"

	arch-meson build --default-library=static --wrap-mode=forcefallback --buildtype=release --debug
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
