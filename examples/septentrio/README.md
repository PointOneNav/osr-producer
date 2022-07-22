# septentrio

This utility accepts SSR and/or OSR corrections from up to three sources,
selects the best, and writes OSR corrections back to a connnected Septentrio
receiver.  The utility communicates to the receiver via a pair of serial ports:
- One port to receive SBF messages containing position, time and ephemeris.
  The utility uses this same port to send corrections and port configuration commands to the receiver.
- Optionaly, another port to receive raw L-band transmissions containing RTCM SSR.

# Polaris OSR Corrections Source

The Polaris OSR source pulls corrections from a local base station server over
an IP network.
```bash
build/examples/septentrio/septentrio \
    -geoid_file=build/external/share/egm2008-15.pgm -sbf_path=/dev/ttyACM0 \
    -sbf_speed=460800 -sbf_interface=USB1 \
    -polaris_osr -polaris_osr_api_key=0123456789
```

# Polaris SSR Corrections Source

The Polaris SSR source is similar to the OSR source, except that we select a
special station that provides SSR instead of OSR.
```bash
build/examples/septentrio/septentrio \
    -geoid_file=build/external/share/egm2008-15.pgm -sbf_path=/dev/ttyACM0 \
    -sbf_speed=460800 -sbf_interface=USB1 \
    -polaris_ssr -polaris_ssr_beacon=SSR22764139040539 \
    -polaris_ssr_api_key=234567890
```

# L-Band SSR Corrections Source

```bash
build/examples/septentrio/septentrio \
    -geoid_file=build/external/share/egm2008-15.pgm -sbf_path=/dev/ttyACM0  \
    -sbf_speed=460800 -sbf_interface=USB1 \
    -lband -lband_path=/dev/ttyACM1 -lband_speed=460800 -lband_interface=USB2 \
    -lband_frequency=1555492500
```
