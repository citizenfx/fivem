---
ns: CFX
apiset: client
game: gta5
---
## SET_PED_TURNING_THRESHOLDS

```c
void SET_PED_TURNING_THRESHOLDS(float min, float max);
```

Purpose: The game's default values for these make shooting while traveling Left quite a bit slower than shooting while traveling right (This could be a game-balance thing?)

Default Min: -45 Degrees
Default Max: 135 Degrees

       \ ,- ~ ||~ - ,
    , ' \    x   x    ' ,
  ,      \    x    x   x  ,
 ,         \  x     x      ,
,            \     x    x  ,
,              \      x    ,
,                \   x     ,
 ,                 \   x x ,
  ,                  \  x ,
    ,                 \, '
      ' - , _ _ _ ,  '  \

If the transition angle is within the shaded portion (x), there will be no transition(Quicker)
The angle corresponds to where you are looking(North on the circle) vs. the heading of your Ped.
Note: For some reason, 

You can set these values to whatever you'd like with this native, but keep in mind that the transitional spin is only clockwise for some reason.

I'd personally recommend something like -135/135

## Parameters
* **min**: Leftside angle on the above diagram
* **max**: Rightside angle on the above diagram