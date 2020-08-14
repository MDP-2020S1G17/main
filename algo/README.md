# Checklist items pertaining to Algo:

(With Arduino)
- [ ] A6/A7: Obstacle avoidance and position recovery (straight line)
  - Robot should be able to detect presence of a single 10x10cm obstacle and take evasive action
  - Robot should return to original line
  - A7 (Extentsion) -- Stop 10-15cm before obstacle and take a diagonal avoidance path

- [ ] B1: Arena exploration simulator
  - Able to demonstrate an algo to explore space of 1.5x2.0m, avoid obstacles and successfully returning eventually to start zone.
  - Must explore ALL **unknown** spaces
  - Should be shown on a simulator displaying a grid map of the arena -- simulator should be able to load grid maps from disk

- [ ] B2: Fastest path computation simulator
  - Able to demonstrate an algo to compute fastest path from start position to goal position -- given a known map of fully explored space of 1.5m x 2.0m
  
- [ ] B3: Generate map descriptor
  - Generate map descriptor for any map
  
- [ ] B4: Time and coverage-limited exploration simulation
  - Demonstrate a timed simulation of time and coverage-aware exploration algorithm:
    1. Simulated robot sould move through the arena exploration simulation at a user-selectable speed of X steps per second 
    2. Demonstrate the automatic termination of your exploration once a user-selected coverage figure (% of maze squares) has bee nachieved
    3. Demonstrate the automatic termination of your exploration once a user-selected time limit (in mins:secs has passed)

- [ ] B5: Extension beyond the basics (With RPi)
  - Robot should be able to generate and display (on a desktop/laptop screen a map of the environment when exploring an enclosed space of size 1.5m x 2.0m)
  - Map updated real time during exploration
  - Display the location of robot as it performs fastest path run
  