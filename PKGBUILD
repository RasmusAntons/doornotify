pkgname=doornotify-git
pkgver=0.0
pkgrel=0
pkgdesc="Doorbell notification provider"
arch=('any')
url="https://github.com/RasmusAntons/doornotify"
makedepends=('git' 'cmake')
depends=('libnotify' 'paho-mqtt-c')
source=("git://github.com/RasmusAntons/doornotify.git")
md5sums=('SKIP')

build() {
	cd doornotify
	git submodule update --init
	mkdir build
	cd build
	cmake ..
	make
}

package() {
	cd doornotify
	install -Dm755 build/doornotify "$pkgdir/usr/bin/doornotify"
	install -Dm644 systemd/doornotify.service "$pkgdir/usr/lib/systemd/user/doornotify.service"
}
