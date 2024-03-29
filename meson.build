project('mcst-fruid', 'c')

cc = meson.get_compiler('c')
cc.find_library('i2c', required: true, has_headers: 'i2c/smbus.h')

systemd = dependency('systemd', required: false)
if systemd.found()
    configure_file(copy: true, install: true,
        input: 'mcst-fruid.service', output: 'mcst-fruid.service',
        install_dir: systemd.get_pkgconfig_variable('systemdsystemunitdir'))
endif

if get_option('spi_backend') == 'auto'
    if target_machine.cpu_family() == 'e2k'
        src_spi = 'spi-ops-spidev.c'
    else
        src_spi = 'spi-ops-mtd.c'
    endif
else
    src_spi = 'spi-ops-' + get_option('spi_backend') + '.c'
endif

src_devtree = ['errors.c', 'dict-ops.c', 'devtree-ops.c', src_spi, 'get-devtree.c']
src_fruid   = ['errors.c', 'dict-ops.c', 'fruid-ops.c', 'i2c-ops.c', 'get-fruid.c']
src_all     = ['errors.c', 'dict-ops.c', 'devtree-ops.c', src_spi, 'fruid-ops.c', 'i2c-ops.c', 'mcst-fruid.c']

libreimu     = dependency('reimu',     fallback : ['libreimu', 'libreimu'])
libreimu_fdt = dependency('reimu_fdt', fallback : ['libreimu', 'libreimu_fdt'])

executable('get-devtree', src_devtree, dependencies: [ libreimu ], install: true, install_dir: 'sbin')
executable('get-fruid',   src_fruid,   dependencies: [ libreimu, libreimu_fdt ], install: true, install_dir: 'sbin')
executable('mcst-fruid',  src_all,     dependencies: [ libreimu, libreimu_fdt ], install: true, install_dir: 'sbin')
