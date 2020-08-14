# Checklist items pertaining to Android:

(With Arduino)
- [ ] C1: Android app is able to transmit / receive text strings over BT serial comms link

- [ ] C2: Functional GUI able to initiate scanning, selection and connection with BT device

- [ ] C3: Functional GUI that provides interactive control of robot movement via BT link
  - Can be regular input, or even touch gestures/tilt, etc.

- [ ] C4: Functional GUI that indicated current status of robot
  - E.g.: Stop/Moving/etc.

- [ ] C5: Functional GUI to enter waypoint & rrobot start coordinates
  - Touch-the-map based 
  - Entering by typing values is **NOT** acceptable
  - Additionally:
    - Need to show X and Y values can be transmitted out via BT serial link and received by AMD tool
    - Show waypoint / virtual robot's graphical position in C6 is updated once coordinates are entered
  
- [ ] C6: 2D display of maze environment and robot's location

- [ ] C7: Functional GUI that provides selection of Manual/Auto updating of graphical display of maze environment
  - Manual: Grid map is updated when user presses an update button
  - Auto: Grid map is refreshed (as a background thread) automatically based on regular interval or change in state of robotic module

- [ ] C8: Functional GUI that provides 2 buttons that support persistent user reconfigurable string commands to the robot
  - Buttons, when pressed send predefined string via BT link
  - Once string is entered, it is persistent or non-volatile
    - If android app is closed & restarted, the reconfigured string will remain and not revert to some default value

- [ ] C9: Rubust connectivity with Bluetooth device
  - Must not hang if connectivity with BT device is temporarily lost -- should re-establish connection once BT device connects with AA again

- [ ] C10: Displaying Number ID Blocks in the Grid Map
  - Obstacle block at a grid location can be replaced with an appropriate numbered block representing an ID number (1-15)

- [ ] C11: Extension beyond the basics.
  - Novel robot movement control AND/OR
    - Demonstrate novel remote motion control of robot using other sensing capabilities of Android tabled (Tilt/continuous touch control)
  - Go beyond basic 2D grid map: Provide button-activated toggle between basic 2D grid map display & 3D first-person-view or 2.5D view of current known map and the robot