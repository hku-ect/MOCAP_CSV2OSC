# MOCAP_CSV2OSC

MOCAP CSV2OSC is a small application that loads a CSV file exported from a Motive recording. The loaded CSV file can then be played back sending it's data through OSC over the netwrok in the same format as the NatNet2OSC bridge.

This application is meant to be used together with a Motive recording so you can work with MOCAP data without having the acces to a MOCAP instalaltion.

# OSC Data Structure

### Rigidbody message: `/rigidbody`

The OSC message consist of a message with with the label /rigidbody and contains 9 values:

- Value 0: ID (int) of rigidbody
- Value 1: Name (string) of rigidbody
- Value 2: X position of rigidbody
- Value 3: Y position of rigidbody
- Value 4: Z position of rigidbody
- Value 5: X rotation of rigidbody (Quarternion)
- Value 6: Y rotation of rigidbody (Quarternion)
- Value 7: Z rotation of rigidbody (Quarternion)
- Value 8: W rotation of rigidbody (Quarternion)

#### Example:

`/rigidbody 0 Blokje2 0.995632 0.025337 0.034089 0.083149 0.995632 0.025337 0.034089`

The units are floats and can either be in meters of centimeters depending on the recording.

### Skeleton message: `/skeleton/<NameOfSkeleton>/<NameOfJoint>`

The OSC message consist of a message with with the label `/skeleton/<NameOfSkeleton>/<NameOfJoint>` and contains 8 values:

- Value 0: Name (string) of the skeleton joint
- Value 1: X position of skeleton joint
- Value 2: Y position of skeleton joint
- Value 3: Z position of skeleton joint
- Value 4: X rotation of skeleton joint (Quarternion)
- Value 5: Y rotation of skeleton joint (Quarternion)
- Value 6: Z rotation of skeleton joint (Quarternion)
- Value 7: W rotation of skeleton joint (Quarternion)

#### Examples:

`/skeleton/Jan/Hip Hip -2.36176 0.856184 -0.603909 0.083149 0.995632 0.025337 0.034089`

`/skeleton/Jan/Ab Ab -2.34777 0.940245 -0.599119 0.071686 0.996859 0.00257 0.033565`

`/skeleton/Jan/Chest Chest -2.31085 1.19635 -0.596527 0.051215 0.987347 -0.149931 0.006673`

`/skeleton/Jan/Neck Neck -2.28746 1.40599 -0.683353 -0.08227 0.911012 -0.404007 -0.008209`

`/skeleton/Jan/Head Head -2.3108 1.51509 -0.774313 0.052667 0.849293 -0.19775 0.486645`

`/skeleton/Jan/LShoulder LShoulder -2.33122 1.3959 -0.655025 0.043044 0.960989 -0.272979 0.011385`

The units are floats and can either be in meters of centimeters depending on the recording.
