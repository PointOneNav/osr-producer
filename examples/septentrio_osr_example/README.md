# Septentrio Example

This application uses the Point One `libosr_producer` library to provide GNSS corrections data to a Septentrio GNSS
receiver connected via serial. It can obtain GNSS corrections from the Point One Polaris network using one or more of
the following sources:
1. GNSS measurements from a nearby base station (observation-space representation data or OSR) sent over IP
2. State-space representation (SSR) model data sent over IP
3. State-space representation (SSR) model data sent over L-band satellite link, received by the Septentrio receiver

The application automatically chooses the best corrections source based on user location and age and quality of the
data, and forwards the corrections as RTCM 10403.3 MSM messages to the receiver.

The application uses the following serial port configuration:
- One port to receive SBF messages containing position, time and ephemeris. The utility uses this same port to send
  corrections and port configuration commands to the receiver.
- A second optional port to receive raw L-band transmissions containing SSR data messages.

## Requirements

The Septentrio must be configured to output the following Septentrio Binary
Format (SBF) messages:
- `PVTGeodetic2` - Position and time updates from the receiver (recommended 1 second interval)
- `GPSNav`, `GLONav`, `GALNav`, `BDSNav` - Satellite ephemeris data (recommended on-change)

To use L-band, the Septentrio should be configured to use the Point One SSR L-band stream, to output the incoming data
on a _separate_ serial interface. The L-band connection details (frequency, service ID, and scrambling vector) will be
provided by Point One.

## Configuring Septentrio Connection Settings

By default, the application looks for a Septentrio connected on `/dev/ttyACM0` at 460800 bits/second. You can change the
settings as follows:

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    --sbf-path=/dev/ttyACM1 --sbf-speed=115200
```

Additionally, if the `--configure` option is enabled (default), the application will attempt to configure the receiver
to output the required SBF message and to connect to the L-band data stream. You may specify the following options to
change this behavior:
- `--configure=all` - Configure both position/ephemeris and L-band settings (default)
- `--configure=none` - Disable configuration
- `--configure=lband` - Configure L-band reception settings
- `--configure=position` - Configure 1 Hz position (PVTGeodetic2) and ephemeris (*Nav)

## Receive OSR Corrections From Polaris

The Polaris OSR source uses Point One's [Polaris Client](https://github.com/PointOneNav/polaris) library to obtain
corrections data from a nearby base station server over an IP network. To use Polaris, you must specify the API key
provided to by Point One:

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    --polaris-osr --polaris-osr-api-key=0123456789
```

> Important: When using your Polaris API key for multiple devices, you must specify unique identifiers for each device
> using the `--polaris-osr-unique-id=NAME` argument. Using the same ID on two devices with the same API key will cause
> unexpected behavior.

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    --polaris-osr --polaris-osr-api-key=0123456789 \
    --polaris-osr-unique-id=my-second-vehicle
```

## Polaris SSR Corrections Source

The Polaris SSR source is similar to the OSR source, except that we select a special data stream that provides SSR data
messages instead of OSR:

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    --polaris-ssr --polaris-ssr-api-key=2345678901 \
    --polaris-ssr-beacon=SSR22764139040539
```

The data stream name (beacon name) will be provided by Point One.

> Note that your SSR API key may differ from your OSR key.

To specify a unique ID for the SSR Polaris connection, use `--polaris-ssr-unique-id`. If unspecified, defaults to
`<ID>_ssr` using the value set by `--polaris-osr-unique-id=ID`.

## L-Band SSR Corrections Source

To receive SSR corrections over L-band, you must configure the Septentrio to receive the L-band signal stream.
L-band connection details (frequency, service ID, and scrambling vector) will be provided by Point One.

```bash
cd build
examples/septentrio_osr_example/septentrio_osr_example \
    --lband --lband-frequency=1555492500
```

By default, the application will attempt to receive L-band data form the Septentrio on `/dev/ttyACM1` at 460800
bits/second.

## Usage Examples

### SSR And OSR Over IP

Connect a Septentrio using `/dev/ttyACM0` (default) and receive both OSR and
SSR corrections data from Polaris over IP:
 ```bash
septentrio_osr_example \
    --polaris-osr --polaris-osr-api-key=0123456789 \
    --polaris-osr-unique-id=my-second-vehicle \
    --polaris-ssr --polaris-ssr-api-key=2345678901 \
    --polaris-ssr-beacon=SSR22764139040539
```

### OSR Over IP And SSR Over L-Band

Connect a Septentrio using `/dev/ttyACM0` (default) and receive OSR over IP and
SSR over L-band from the receiver:
 ```bash
septentrio_osr_example \
    --polaris-osr --polaris-osr-api-key=0123456789 \
    --polaris-osr-unique-id=my-second-vehicle \
    --lband --lband-frequency=1555492500
```

### Configure Septentrio L-Band Settings

Configure required settings for a Septentrio connected on `/dev/ttyACM3`:

```bash
septentrio_osr_example --sbf-path=/dev/ttyACM3 --configure=lband
```
