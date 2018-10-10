# DnD20 - Oct 9 2018

* To model a regular icosohedron in FreeCAD, what you do is make 3 perpendicular golden rectangles, then connect their corners with edges.


![Golden Rectangles](https://github.com/MatthewCLind/DnD20/blob/master/MCAD/screenshots/golden-rects.png)


* Then you turn the edges into faces, the faces into a shell, and the shell into a solid
* To hollow out your solid, you can duplicate it and use the Draft toolbar to scale the solid down. This gives you the inner solid to do a boolean cut with

I think later this week I'll make a cardboard prototype so that I have the actual physical device put together. That way I can save off some of the accelerometer data and see what it looks like while rolled, too.