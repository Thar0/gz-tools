# gz tools

A collection of tools relating to [gz, the OoT Practice ROM](https://github.com/glankk/gz "gz, the OoT Practice ROM"), mostly tools for manipulating gz macro (`.gzm`) files.

It provides the following tools:

### gzmstat

A simple tool for reading various data in a .gzm file.

Example usage: `./gzmstat macro.gzm`

Example output:
```
macro.gzm:
n_input: 450
n_seed: 4
n_oca_input: 0
n_oca_sync: 0
n_room_load: 0
rerecords: 29
last_recorded_frame: 449
gzm has 4 seeds:
  frame: 84, old: 7860f5c3, new: 840aaf88
  frame: 177, old: ecdafc54, new: 5876bcf8
  frame: 303, old: 3045b7e3, new: faf4f6d2
  frame: 448, old: 32eef46a, new: b304498c
```

 - `n_input` is the number of stored inputs, one per frame of the macro. This also doubles as the duration.
 - `n_seed` is the number of saved rng seeds. Each saved rng seed stores three numbers: The frame on which the rng seed should be set, the value the rng seed should be before restoring the saved value, and finally the saved value itself.
 - `n_oca_input`, `n_oca_sync`, `n_room_load` are additional data optionally stored by macros to help movies sync under certain conditions such as playing the ocarina.
 - `rerecords` and `last_recorded_frame` are the number of rerecords and the frame that was last recorded, used to track when to increment the rerecord counter.

### gzmcat

Concatenates two separate macro files together into a single macro file. The two macros are concatenated in such a way that the rng remains synced throughout.
In order to do this the first macro must end shortly after entering a loading zone, and the second macro must start just before entering the same loading zone. Concatenating these two macros will stitch them on the frame the scene loads and fix the saved rng values and frame numbers.

Example usage: `./gzmcat macro1.gzm macro2.gzm macro3.gzm`
(with `macro1.gzm` and `macro2.gzm` satisfying the condition outlined above)

### gzmslice

Slices a piece of the input macro from the input starting frame to the input ending frame into a new macro file. 

Example usage: `./gzmslice input.gzm output.gzm 0 2000`