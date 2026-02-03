Terminal 1:

cd ~/ros2_ws

colcon build

source install/setup.bash




Terminal 2:

source ~/ros2_ws/install/setup.bash

ros2 launch my_robot_description display.launch.py





Terminal 3:

source ~/ros2_ws/install/setup.bash

ros2 run my_robot_sim effects_driver




RViz Display Checklist:
Remove Axes to have robot model more visible
Desc Topic: /robot_description
Panels: Grid, RobotModel, Timer, Marker (Topic: /led_marker), TF




Functional Commands:

Forward:

source ~/ros2_ws/install/setup.bash

ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.2}, angular: {z: 0.0}}" -r 10




Turn left:

ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.0}, angular: {z: 0.8}}" -r 10




Stop:

ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.0}, angular: {z: 0.0}}" -r 10
