# dcf77_dec
**dcf77_dec** is a software decoder for the German time signal [DCF77](https://en.wikipedia.org/wiki/DCF77) broadcasted from [Mainflingen](https://www.openstreetmap.org/way/94232112#map=16/50.01499/9.00875) near Frankfurt. The decoding process is based on [Goertzel's algorithm](https://en.wikipedia.org/wiki/Goertzel_algorithm) which determines how much of a certain frequency is present in a block of audio samples.

## Compilation and installation
1. Enter the directory of `dcf77_dec`
2. Build `dcf77_dec`
```bash
make
```
3. Install `dcf77_dec`
```bash
sudo make install
```

## Usage
Make sure that the audio signal of DCF77 is already audible before starting `dcf77_dec`!<br/>
By default `dcf77_dec` uses the *ALSA* interface `pulse` to receive the signal.<br/>

**Launch**
```bash
dcf77_dec
```

Now the program first tries to find the frequency and the level of the signal.<br/>
After that it waits for the sync signal (1.8 seconds high) until it starts receiving and decoding the DCF77 signal.