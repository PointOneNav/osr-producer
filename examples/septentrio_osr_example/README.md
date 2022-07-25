# Septentrio Example

This utility accepts SSR and/or OSR corrections from up to three sources,
selects the best, and writes OSR corrections back to a connnected Septentrio
receiver.  The utility communicates to the receiver via a pair of serial ports:
- One port to receive SBF messages containing position, time and ephemeris.
  The utility uses this same port to send corrections and port configuration commands to the receiver.
- Optionaly, another port to receive raw L-band transmissions containing RTCM SSR.

## Polaris OSR Corrections Source

The Polaris OSR source pulls corrections from a local base station server over
an IP network.

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    -sbf-path=/dev/ttyACM0 -sbf-speed=460800 -sbf-interface=USB1 \
    -polaris-osr -polaris-osr-api-key=0123456789
```

> Important: When using your Polaris API key for multiple devices, you must specify unique identifiers for each device
> using the `--polaris-osr-unique-id=NAME` argument. Using the same ID on two devices with the same API key will cause
> unexpected behavior.

## Polaris SSR Corrections Source

The Polaris SSR source is similar to the OSR source, except that we select a
special station that provides SSR instead of OSR.

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    -sbf-path=/dev/ttyACM0 -sbf-speed=460800 -sbf-interface=USB1 \
    -polaris-ssr -polaris-ssr-beacon=SSR22764139040539 \
    -polaris-ssr-api-key=234567890
```

## L-Band SSR Corrections Source

To receive SSR corrections over L-band, you must configure the Septentrio to receive the L-band signal stream.
L-band connection details (frequency, service ID, and scrambling vector) will be provided by Point One.

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    -sbf-path=/dev/ttyACM0 -sbf-speed=460800 -sbf-interface=USB1 \
    -lband -lband-path=/dev/ttyACM1 -lband-speed=460800 -lband-interface=USB2 \
    -lband-frequency=1555492500
```
