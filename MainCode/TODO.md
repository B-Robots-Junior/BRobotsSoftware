1. reset interrupt for resetting to checkpoint (FINISHED)
2. make black tile interrupt faster
3. possibly optimize the mapper action list (done) !!NOT IMPLEMENTED!!
4. Update the gyro code to bno055 (done) !!NOT IMPLEMENTED!!
5. Possibly force a tof update on tile detections to stop edge cases of the back tofs not updating (FIXED, wrong problem)
6. mapping reset is not optimal, because when you reset it may not finish the entire maze, because the tiles are marked as detected and won't be driven on again (FIXED) !!NOT IMPLEMENTED!!
7. Test Bumper correction (done)
8. Lötbrücke für die leds (done)
9. add buttons for calib, start and reset (start and reset is done, calibration is currently eeprom)
10. implement camera code
11. add bumper
12. auswurfsystem