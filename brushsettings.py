"Brush Settings (source for generated code, but also imported at runtime)"

inputs_list = [
    # name, default maximum, hard maximum, tooltip (TODO)
    ['pressure', 1.0, True, "The pressure reported by the tablet, between 0.0 and 1.0. If you use the mouse, it will be 0.5 when a button is pressed and 0.0 otherwise."],
    ['speed', 5.0, False, "How fast you currently move. This can change very quickly."],
    ['speed2', 5.0, False, "Same as speed, but changes slower. Also look at the 'speed2 slowness' setting."],
    ['random', 1.0, True, "Random noise, changing at each evaluation."],
    # ...
    ]

settings_list = [
    # internal name, displayed name, special, minimum, default, maximum, tooltip
    ['opaque', 'opaque', None, 0.0, 1.0, 1.0, "0 means brush is transparent, 1 fully visible"],
    ['opaque_multiply', 'opaque multiply', None, 0.0, 0.0, 1.0, "This gets multiplied with opaque, and in fact it does not matter whether you change opaque or opaque_multiply. I suggest to use this one for the pressure mapping, and let the normal opaque be constant."],
    ['radius_logarithmic', 'radius', None, -0.5, 2.0, 5.0, "basic brush radius (logarithmic)\n 0.7 means 2 pixels\n 3.0 means 20 pixels"],
    ['hardness', 'hardness', None, 0.0, 1.0, 1.0, "hard brush-circle borders (setting to zero will draw nothing; it's not implemented like in GIMP, I haven't figured out yet)"],
    ['dabs_per_basic_radius', 'dabs per basic radius', None, 0.0, 0.0, 5.0, "dabs to draw while the pointer moves one brush radius\nthis is a constant, if you make it depend on something (pressure, speed, whatever) this will be ignored"],
    ['dabs_per_actual_radius', 'dabs per actual radius', None, 0.0, 2.0, 5.0, "same as above, but the radius actually drawn is used, which might change dynamically\nthis is a constant, if you make it depend on something (pressure, speed, whatever) this will be ignored"],
    ['dabs_per_second', 'dabs per second', None, 0.0, 0.0, 80.0, "dabs to draw each second, no matter how far the pointer moves\nthis is a constant, if you make it depend on something (pressure, speed, whatever) this will be ignored"],
    ['radius_by_random', 'radius by random', None, 0.0, 0.0, 10.0, "alter the radius randomly each dab\n 0.0 disable\n 0.7 biggest radius is twice as large as smallest\n 3.0 biggest radius 20 times as large as smallest"],
    ['speed2_slowness', 'speed2 slowness', None, 0.0, 1.0, 10.0, "how slow the input speed2 is following the real speed\n0.0 change immediatly as your speed changes"],
    ['offset_by_random', 'offset by random', None, 0.0, 0.0, 10.0, "add randomness to the position where each dab is drawn\n 0.0 disabled\n 1.0 standard derivation is one radius away (as set above, not the actual radius)"],
    ['offset_by_speed', 'offset by speed', None, -30.0, 0.0, 30.0, "change position depending on pointer speed\n= 0 disable\n> 0 draw where the pointer moves to\n< 0 draw where the pointer comes from"],
    ['saturation_slowdown', 'saturation slowdown', None, -1.0, 0.0, 1.0, "When painting black, it soon gets black completely. This setting controls how fast the final brush color is taken:\n 1.0 slowly\n 0.0 disable\n-1.0 even faster\nThis is nolinear and causes strange effects when it happens too fast. Set occupancy low enough to avoid this.\nFor example, a full-occupancy black stroke might get brighter over grey areas than over white ones.\nFIXME: this setting seems not to work as I expected. I reccomend to set it to zero for anything else than black/white drawing."],
    ['position_T', 'slow position', None, 0.0, 0.0, 10.0, "Slowdown pointer tracking speed. 0 disables it, higher values remove more jitter in cursor movements. Useful for drawing smooth, comic-like outlines."],
    ['position_T2', 'slow position 2', None, 0.0, 0.0, 10.0, "Similar as above but at brushdab level (ignoring how much time has past, if brushdabs do not depend on time)"],
    ['color_value', 'color brightness', None, -2.0, 0.0, 2.0, "change the color brightness (also known as intensity or value) depending from the choosen color\n-1.0 darker\n 0.0 disable\n 1.0 brigher"],
    ['color_saturation', 'color saturation', None, -2.0, 0.0, 2.0, "change the color saturation\n-1.0 more grayish\n 0.0 disable\n 1.0 more saturated"],
    ['color_hue', 'color hue', None, -2.0, 0.0, 2.0, "change color hue\n-1.0 clockwise color hue shift\n 0.0 disable\n 1.0 counterclockwise hue shift"],
    ['adapt_color_from_image', 'adapt color from image', None, 0.0, 0.0, 1.0, "slowly change the color to the one you're painting on (some kind of smudge tool)\nNote that this happens /before/ the hue/saturation/brighness adjustment below: you can get very different effects (eg brighten image) by combining with them."],
    ]

class BrushInput:
    pass

inputs = []
for i_list in inputs_list:
    i = BrushInput()
    i.name, i.max, i.hardmax, i.tooltip = i_list
    i.index = len(inputs)
    inputs.append(i)

class BrushSetting:
    pass

settings = []
for s_list in settings_list:
    s = BrushSetting()
    s.cname, s.name, s.special, s.min, s.default, s.max, s.tooltip = s_list
    s.index = len(settings)
    settings.append(s)
