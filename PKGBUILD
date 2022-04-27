pkgname=iptsd
pkgver=0.5.1
pkgrel=1
pkgdesc='Userspace daemon for Intel Precise Touch & Stylus'
arch=('x86_64')
url='https://github.com/linux-surface/iptsd'
license=('GPL')
depends=('libinih')
makedepends=('meson')

build() {
	cd $startdir

	arch-meson . build
	meson compile -C build
}

check() {
	cd $startdir

	meson test -C build
}

package() {
	cd $startdir

	DESTDIR="$pkgdir" meson install -C build
}
