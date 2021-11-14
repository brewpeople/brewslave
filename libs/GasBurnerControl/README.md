# Gas Burner Control

Arduino based state machine for the gas burner controller (GBC)
`Satronic 812.2 Mod. 10` in combination with remote reset `Satronic FR 870`.


## Disclaimer

Use at own risk! Automatic ignition of a gas burner may lead to mayhem
(uncontrolled gas leakage, explosion, etc.). Hence, I neither guarantee
functionality of this library nor am I responsible for any accidents if you
decide to utilize it.


## Working Principle

The GBC is intended to be powered with a relay controlled by an Arduino. Since
the GBC has an interal error detection and a manual reset button, this button
is controlled by a second relay allowing programmatic unlocking (called
dejam(ing) in the code).

When GBC detects an error, it can be dejammed (state `GBC_DEJAM`) only after
"approx. 60 s" according to the datasheet. In addition, the reset button can
only be activated every "approx. 10 s". If it runs as inteded, the ignition
(state `GBC_IGNITION`) takes 20 s until the GBC controls whether the flame is
truly burning (state `GBC_RUNNING`) or if it locks (state `GBC_DEJAM`).

The library controls a defined number of dejam and ignition attempts and, if
not successful, powers down the GBC (state `GBC_ERROR`).


## State Machine

### `GBC_IDLE`

The GBC is off and doing nothing.


### `GBC_STARTING`

As I was unsure if there is a starting delay after switching the relay, this
state adds a certain delay after powering up the GBC.


### `GBC_IGNITION`

Checks if ignition is in progress. Changes either to `GBC_DEJAM` as soon
as an error occurs or to `GBC_RUNNING` when ignition was successful after the
GBS's internal ignition timer.


### `GBC_DEJAM`

The GBC may raise an error either when ignition was unsuccessful after 20 s
or when something is wrong with the power supply. Since the GBC retains its
locked/jammed state even when powered off, a GBC-error may occur immediately
after powering up, skipping the ignition. in this case the library tries
to dejam it immediately by "pressing the reset button", which is equivalent to
powering up the relay of the remote reset for a short time. Similar to the
`GBC_STARTING` state, I was not sure how long it takes to unlock/dejam, a
small delay after each dejam attempt is included in the code.

This state is somewhat a linear state machine on its own realized in the
private function `_dejam(unsigned int delay_s)`.
* Wait pre dejam time (0 s after power-up, 60 s after failed ignition,
or 10 s for conescutive dejam attempts)
* Press reset button
* Wait for `DEJAM_DURATION`
* Release reset button
* Wait for `POST_DEJAM_DELAY`

I refrained from naming these substates.

If this dejam attempt was not successful as well as if the lock/jam occured
after ignition was running for some time, the next dejam attempt occurs only
after "approx. 60 s" in line with the datasheet information. If unsuccessful,
consecutive dejam attempts will be carried out every "approx. 10 s" until
it either works or a predefined number of unsuccessful attempts is reached.

Simmilarly, ignition attempts (when ignition started but did not continue to
running mode --> GBC detects no burning flame), also have a predefined
max. counter.


### `GBC_ERROR`

This state is reached when either a predefined number of consecutive dejam
attempts failed, or when a consecutive number of ignition attempts failed.
Whatever occurs first. In this state the GBC is powered down but the
state machine remains in this state until other user interaction was conducted.


## Usage

Call constructor at the beginning of the main code (before `setup` or `loop`) `
GFAStateMachine gbc(3, 4, 5, 6, 7);`

To start the GBC call `gbc.start();`

As the library is inteded to be interrupt free and non-blocking, call
`gbc.update()` as often as possible in `loop`. Any state change and error
detection will only be performed by calling this function!

Finally, if you want to stop the Bruner/GBC call `gbc.stop()`

Thats it :)


If you want to get the current state of the state machine without updating,
call `gba.getState()`.


## Options

```
#define GBC_N_DEJAM_ATTEMPTS 3      // number of unsuccessful dejam attempts before aborting permanently (1-255)
#define GBC_N_IGNITION_ATTEMPTS 3   // number of unsuccessful ignition attempts before aborting permanently (1-255)

#define GBC_HIGH 0                  // logical high (0: GND, 1: VCC)
#define GBC_LOW 1                   // logical low (0: GND, 1: VCC)

#define GBC_SERIAL_DEBUG            // if uncommented every update will also send a message over serial (uses lots of RAM)


/* TIMER SETTINGS */
#define GBC_START_DELAY 2           // s, initial delay after powering the GBC
#define GBC_DEJAM_DELAY 65          // s, wait time until dejamming is possible
#define GBC_DEJAM_DELAY2 10         // s, wait time between additional dejam attempts
#define GBC_IGNITION_DELAY 22       // s, time after which ignition should be complete
#define GBC_DEJAM_DURATION 1000     // ms, duration of button press during dejamming
#define GBC_POST_DEJAM_DELAY 1000   // ms, delay after dejam button release
```


## Wiring

TBD


## ToDo
- [ ] Let someone with programming skills review my code :D
- [x] Check if all the `this->` pointers to library variables are necessary.
- [ ] Add library and argument to `./configure` script. (Shall/can we pass all
five required pins as argument similar to `--add-ds18b20`?
- [ ] Test all states and transitions with hardware.
- [ ] Some condition clauses are obsolete because they should not be reachable.
I left them in the code for now. Should be cleaned up after field test.


## Possible Enhancements
- [ ] Differentiate between different error states? You may land ind `GBC_ERROR`
but there is no feedback as to why this state was reached.
    - [ ] Error if ignition never started
    - [ ] Error if ignition was attempted but unsuccessful
    - [ ] Error if unplausible state was reached -> likely wrong wiringg
- [ ] Make counter for dejam and ignition attempts readable from outside?




