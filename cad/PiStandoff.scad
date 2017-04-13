
translate([15,5,0]) Standoff();
translate([5,5,0]) Standoff();
translate([15,-5,0]) Standoff();
translate([5,-5,0]) Standoff();

translate([-5,5,0]) Standoff();
translate([-15,5,0]) Standoff();
translate([-5,-5,0]) Standoff();
translate([-15,-5,0]) Standoff();

translate([15,25,0]) Standoff();
translate([5,25,0]) Standoff();
translate([15,15,0]) Standoff();
translate([5,15,0]) Standoff();

translate([-5,25,0]) Standoff();
translate([-15,25,0]) Standoff();
translate([-5,15,0]) Standoff();
translate([-15,15,0]) Standoff();

module Standoff()
{
	difference()
	{
		cylinder(h=12.15, r=6.5/2, $fn=70);
		cylinder(h=100, r=3/2, $fn=70);
	}
}



