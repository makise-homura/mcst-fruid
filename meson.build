project('mcst-fruid', 'c')

exe_cflags = []
cc = meson.get_compiler('c')
cc.find_library('i2c', required: true, has_headers: 'i2c/smbus.h')

src = ['errors.c', 'devtree-ops.c', 'dict-ops.c', 'fruid-ops.c', 'i2c-ops.c', 'mcst-fruid.c']

if get_option('spi_backend') == 'auto'
    if target_machine.cpu_family() == 'e2k'
        src += 'spi-ops-spidev.c'
    else
        src += 'spi-ops-mtd.c'
    endif
else
    src += 'spi-ops-' + get_option('spi_backend') + '.c'
endif

systemd = dependency('systemd', required: false)
if systemd.found()
    configure_file(copy: true, install: true,
        input: 'mcst-fruid.service', output: 'mcst-fruid.service',
        install_dir: systemd.get_pkgconfig_variable('systemdsystemunitdir'))
endif

executable('mcst-fruid', src, install: true, install_dir: 'libexec')
