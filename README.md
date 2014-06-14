YMZ294-sequencer
================

Simple chiptune sequencer for Arduino and Yamaha YMZ294 soundchip.

Currently implemented:
- Channels.
- Note positions and tone.
- Glide speed for every note.
- 2 to 4 note arpegiator. Speed and pitches can be specified for each notes.

melody settings is currently hard-coded.

# YMZ294 connections

Connect the DATA input of YMZ294 to pin 0-7 of arduino.
A LED can be connected to pin 8 of arduino and will flash every beat.

## Block diagram

```
                φM                   WR   CS   A0       D0 ~ D7     IC  TEST      
         .......██...................██...██...██........████.......██...██.......
         .       │                    │    │    │         ││         │    │      .
         .       │          ┌─────────│────│────│─────────┘│         │    │      .
         .       │          │┌────────│────│────│─────────┐│         ▼    ▼      .
         .       │          ││        │    │    │         ││                     .
         .  ┌────▼────┐     ││  ┌─────▼────▼────▼─────┐   ││                     .
         .  │frequency│     ││  │ bus control decoder │   ││                     .
    4/8 ██──> divider │     ││  └─────┬─────────┬─────┘   ││                     .
         .  └────┬────┘     ││        │         │         ││                     .
         .       │       ┌──▼▼────────▼──┐   ┌──▼─────────▼▼────┐                .
         .       ▼       │register adress├───>  register array  │                .
         . master clock  │     latch     │   │                  │                .
         .    (2MHz)     └───────────────┘   └──────────┬───────┘                .
         .       ┌────────────┬────────────┬────────────┼────────────┐           .
         .       │            │            │            │            │           .
         .  ┌────▼────┐  ┌────▼────┐  ┌────▼────┐  ┌────▼────┐  ┌────▼────┐      .
         .  │  noise  │  │ A  tone │  │ B  tone │  │ C  tone │  │enveloppe│      .
         .  │generator│  │generator│  │generator│  │generator│  │generator│      .
         .  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘      .
         .       └─────────┬──│─────────┬──│─────────┐  │            │           .
         .                 │  │         │  │         │  │            │5 bit      .
         .               ┌─▼──▼────┐  ┌─▼──▼────┐  ┌─▼──▼────┐       │logaritmic .
         .               │  mixer  │  │  mixer  │  │  mixer  │       │resolution .
         .               └────┬────┘  └────┬────┘  └────┬────┘       │           .
         .                    │  ┌─────────│──┬─────────│──┬─────────┘           .
         .                    │  │         │  │         │  │                     .
         .               ┌────▼──▼─┐  ┌────▼──▼─┐  ┌────▼──▼─┐                   .
         .               │   VCA   │  │   VCA   │  │   VCA   │                   .
         .               └────┬────┘  └────┬────┘  └────┬────┘                   .
         .                    │            │            │                        .
         .                    └─────────── + ───────────┘                        .
         .                                 │                                     .
         .................................██......................................
                                          S0                                      
```

## Pinout diagram

```

                                              ┌────┬─┬────┐
  PIN11 ───────┬─────> write enable ────> /WR─┤1 ● └─┘  18├─D0  ┐ LSB
    +5V ──>    └─────>  chip select ────> /CS─┤2        17├─D1  │
  PIN12 ──> address / data selector ────>  A0─┤3   YMZ  16├─D2  │
                                +5V ────> VDD─┤4   294  15├─D3  ├─ DATA <─── Arduino PIN 0-7
                      analog output <────  S0─┤5        14├─D4  │            or Shift register
                                GND ────> GND─┤6        13├─D5  │
    crystal oscillator output──>CLK ────>  φM─┤7        12├─D6  │
           CLK RATE (H=4MHz, L=8MHz)────> 4/8─┤8        11├─D7  ┘ MSB
         +5V(or floating) ──> reset ────> /IC─┤9        10├─/TEST <──── test <── +5V (or floating)
                                              └───────────┘

                    ┌───────┐
                Vcc─┤8     5├─OUTPUT
                    │  C012 │
                 NC─┤1●    4├─GND
                    └───────┘
            (C012 crystal oscillator)
 
 
```

## Register array functions

```

                           MSB                                 LSB
 ┌────────────────────────┬────┬────┬────┬────┬────┬────┬────┬────┐
 │         register  @ADR │ D7 │ D6 │ D5 │ D4 │ D3 │ D2 │ D1 │ D0 │
 ├────────────────────────┼────┴────┴────┴────┴────┴────┴────┴────┤ Fmaster: master clock frequency (2MHz)
 │                     $0 │           fine tune (8 bit)           │ 
 ├ Channel A ─ ─ ─ ─ ─────┼────┬────┬────┬────┬───────────────────┤ 
 │                     $1 │ ░░ │ ░░ │ ░░ │ ░░ │   pitch (4 bit)   │ tone frequency: Ft
 ├────────────────────────┼────┴────┴────┴────┴───────────────────┤ Ft = Fmaster / (32 * (pitch << 8 + fine))
 │                     $2 │           fine tune (8 bit)           │ 
 ├ Channel B ─ ─ ─ ─ ─────┼────┬────┬────┬────┬───────────────────┤ -> To get t (12 bit) from frequency Ft:
 │                     $3 │ ░░ │ ░░ │ ░░ │ ░░ │   pitch (4 bit)   │ t = Fmaster / (32 * Ft)
 ├────────────────────────┼────┴────┴────┴────┴───────────────────┤ t : [1..4095] -> Ft : [62500Hz .. 15.26Hz]
 │                     $4 │           fine tune (8 bit)           │ (t = 0 means no sound)
 ├ Channel C ─ ─ ─ ─ ─────┼────┬────┬────┬────┬───────────────────┤
 │                     $5 │ ░░ │ ░░ │ ░░ │ ░░ │   pitch (4 bit)   │
 ├────────────────────────┼────┼────┼────┼────┴───────────────────┤ noise frequency: Fn
 │ Noise channel       $6 │ ░░ │ ░░ │ ░░ │    noise tune (5 bit)  │ Fn = Fmaster / 32 * tune    (tune:[0..31])
 ├────────────────────────┼────┼────┼────┼────┬────┬────┬────┬────┤
 │ Mixer               $7 │ ░░ │ ░░ │ C  │ B  │ A  │ C  │ B  │ A  │ `0` enable sound output
 │                        │ ░░ │ ░░ │-----noise----│-----tone-----│ noise is mixed with tone if both are set to `0`
 ├────────────────────────┼────┼────┼────┼────┼────┴────┴────┴────┤ 
 │              ch.A   $8 │ ░░ │ ░░ │ ░░ │ M  │  volume (4 bit)   │ if M = 0 then constant amplification
 ├             ───────────┼────┼────┼────┼────┼───────────────────┤               of `volume` is applied 
 │ Volume CTRL  ch.B   $9 │ ░░ │ ░░ │ ░░ │ M  │  volume (4 bit)   │
 ├             ───────────┼────┼────┼────┼────┼───────────────────┤ if M = 1 then enveloppe is used for
 │              ch.C   $A │ ░░ │ ░░ │ ░░ │ M  │  volume (4 bit)   │               amplification
 ├────────────────────────┼────┴────┴────┴────┴───────────────────┤
 │                     $B │                                    LSB│ 
 ├ Enveloppe ─ ─ ─ ─ ─────┼─  -  -  -   EP  (16 bit) -  -  -  -  ─┤ enveloppe time = 256 * EP / Fmaster 
 │                     $C │MSB                                    │
 ├────────────────────────┼────┬────┬────┬────┬────┬────┬────┬────┤
 │ Enveloppe shape     $D │ ░░ │ ░░ │ ░░ │ ░░ │CONT│ATT │ALT │HOLD│ there is a total of 8 shapes (see below)
 └────────────────────────┴────┴────┴────┴────┴────┴────┴────┴────┘
```

## Enveloppe shapes

```
dec     bin
0~3     00xx ＼_____
         
4~7     01xx ／_____
         
8       1000 ＼＼＼＼
         
9       1001 ＼_____  (same as 00xx)
         
10      1010 ＼／＼／
               _____
11      1011 ＼
         
12      1100 ／／／／
               _____ 
13      1101 ／
         
14      1110 ／＼／＼
         
15      1111 ／_____  (same as 01xx)
```
