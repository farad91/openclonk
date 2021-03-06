/**
 Library for shapes
 See docs/sdk/script/Shape.xml for documentation.
 
 @author Sven2
 
 */

/* Base shape */

local BaseShape;

/* Rectangle shape */

local BaseRectangle; // properties x,y,w,h

// Point contained in rectangle?
private func BaseRectangle_IsPointContained(int x, int y)
{
	//return ((x-this.x+this.w)/this.w) * ((y-this.y+this.h)/this.h) == 1 && x>=this.x;
	return x>=this.x && y>=this.y && x<this.x+this.w && y<this.y+this.h;
}

// bounding rectangle is just self
private func BaseRectangle_GetBoundingRectangle() { return this; }

private func BaseRectangle_Find_In(context)
{
	if (!context) context = Global;
	return context->Find_InRect(this.x, this.y, this.w, this.h);
}

private func BaseRectangle_Find_At(context)
{
	if (!context) context = Global;
	return context->Find_AtRect(this.x, this.y, this.w, this.h);
}

private func BaseRectangle_GetRandomPoint(proplist result)
{
	if (this.w<=0 || this.h <=0) return false;
	result.x = this.x + Random(this.w);
	result.y = this.y + Random(this.h);
	return true;
}

/** Constructor of rectangle area. (x,y) is included; (x+w,y+h) is excluded.
 @par x Global left side of rectangle
 @par y Global top side of rectangle
 @par w Rectangle width
 @par h Rectangle height
 @return return a shape proplist representing a rectangle
*/
public func Rectangle(int x, int y, int w, int h)
{
	return new BaseRectangle { x=x, y=y, w=w, h=h };
}


/* Circle shape */

local BaseCircle; // properties cx,cy,xr,yr

// point contained in rectangle?
private func BaseCircle_IsPointContained(int x, int y)
{
	x-=this.cx; y-=this.cy;
	var r=this.r;
	return x*x + y*y <= r*r;
}

// bounding rectangle
private func BaseCircle_GetBoundingRectangle()
{
	var r=this.r;
	return new Shape.BaseRectangle { x=this.cx-r, y=this.cy-r, w=r*2+1, h=r*2+1 };
}

private func BaseCircle_GetRandomPoint(proplist result)
{
	// Make sure radius circles are weighed equally
	var r2 = this.r * this.r + 1;
	if (r2>0x7fff) // for large numbers, the random function doesn't work
		r2 = Random(0x8000) + Random(r2/0x8000+1) * 0x8000;
	else
		r2 = Random(r2);
	var r = Sqrt(r2), a = Random(360);
	result.x = this.cx + Sin(a, r);
	result.y = this.cy + Cos(a, r);
	return true;
}

/** Constructor of circular area.
 @par cx Global center x of circle
 @par cy Global center y of circle
 @par r Circle radius
 @return return a shape proplist representing a filled circle around a point
*/
public func Circle(int cx, int cy, int r)
{
	return new BaseCircle { cx=cx, cy=cy, r=r };
}


/* Intersection */

local BaseIntersection;

private func BaseIntersection_IsPointContained(int x, int y)
{
	// Intersection: If any of the sub-areas exclude the point, then it's excluded
	for (var area in this.areas)
		if (!area->IsPointContained(x, y))
			return false;
	return true;
}

private func BaseIntersection_GetBoundingRectangle()
{
	// Bounding rectangle of intersection
	var result, rt;
	for (var area in this.areas)
		if (rt = area->GetBoundingRectangle())
		{
			// first bounds determine area
			if (!result)
			{
				result = new Shape.BaseRectangle {x = rt.x, y = rt.y, w = rt.w, h = rt.h};
			}
			else
			{
				// following bounds reduce area
				if (rt.x + rt.w < result.x + result.w) result.w = Max(rt.x + rt.w - result.x);
				if (rt.y + rt.h < result.y + result.h) result.h = Max(rt.y + rt.h - result.y);
				if (rt.x > result.x)
				{
					result.w = Max(result.w - rt.x + result.x);
					result.x = rt.x;
				}
				if (rt.y > result.y)
				{
					result.h = Max(result.h - rt.y + result.y);
					result.y = rt.y;
				}
			}
		}
	return result;
}

private func BaseIntersection_GetRandomPoint(proplist result, int num_tries)
{
	// The precise shape of the intersection is unknown. So try 100 times to get a location from any of the subsections
	if (!GetLength(this.areas)) return false;
	if (!num_tries) num_tries = 200;
	while (num_tries>0)
	{
		for (var area in this.areas)
		{
			// get point from subsection
			if (!area->GetRandomPoint(result, num_tries)) return false; // sub-area empty? (note num-tries goes down)
			var pt_ok = true;
			// ensure it's contained in all other subsections
			for (var area2 in this.areas) if (area != area2)
				if (!area2->IsPointContained(result.x, result.y))
				{
					pt_ok = false;
					break;
				}
			if (pt_ok) return true;
			--num_tries;
		}
	}
	return false;
}

/** Constructor of intersection area.
 @par c1, c2, ... Up to ten parameters for intersected areas.
 @return return a shape proplist representing a shape that contains only the points included in all the passed sub-shapes.
*/
public func Intersect(proplist c1, proplist c2, ...)
{
	// Intersection of one area?
	if (!c2) return c1;
	// Otherwise, built array
	var areas = [c1, c2], i=1, area;
	while (area = Par(++i)) areas[i] = area;
	return new BaseIntersection { areas = areas };
}


/* Combination */

local BaseCombination;

private func BaseCombination_IsPointContained(int x, int y)
{
	// Combination: If any of the sub-areas include the point, then it's included
	for (var area in this.areas)
		if (area->IsPointContained(x, y))
			return true;
	return false;
}

private func BaseCombination_GetBoundingRectangle()
{
	// Bounding rectangle of combination
	var result, rt;
	for (var area in this.areas)
		if (rt = area->GetBoundingRectangle())
		{
			// first bounds determine area
			if (!result)
			{
				result = new Shape.BaseRectangle {x = rt.x, y = rt.y, w = rt.w, h = rt.h};
			}
			else
			{
				// following bounds enlarge area
				if (rt.x + rt.w > result.x + result.w) result.w = rt.x + rt.w - result.x;
				if (rt.y + rt.h > result.y + result.h) result.h = rt.y + rt.h - result.y;
				if (rt.x < result.x)
				{
					result.w += result.x - rt.x;
					result.x = rt.x;
				}
				if (rt.y < result.y)
				{
					result.h += result.y - rt.y;
					result.y = rt.y;
				}
			}
		}
	return result;
}

private func BaseCombination_GetRandomPoint(proplist result, int num_tries)
{
	// Just get a random point from a random subsection
	// Note that this doesn't weigh areas equally.
	var len = GetLength(this.areas);
	if (!len) return false;
	return this.areas[Random(len)]->GetRandomPoint(result, num_tries);
}

/** Constructor of combined area.
 @par c1, c2, ... Up to ten parameters for combined areas.
 @return return a shape proplist representing a shape that contains all the points included in any of the passed sub-shapes.
*/
public func Combine(proplist c1, proplist c2, ...)
{
	// Combination of one area?
	if (!c2) return c1;
	// Otherwise, built array
	var areas = [c1, c2], i=1, area;
	while (area = Par(++i)) areas[i] = area;
	return new BaseCombination { areas = areas };
}


/* Subtraction */

local BaseSubtraction;

private func BaseSubtraction_IsPointContained(int x, int y)
{
	// Subtraction contains everything in "in" area that is not contained in "ex" area
	return this.in->IsPointContained(x, y) && !this.ex->IsPointContained(x, y);
}

private func BaseSubtraction_GetBoundingRectangle()
{
	// Simply use "in" area since it bounds everything
	return this.in->GetBoundingRectangle();
}

private func BaseSubtraction_GetRandomPoint(proplist result, int num_tries)
{
	// Get random point in in-area until it lies outside ex-area
	if (!num_tries) num_tries = 200;
	while (num_tries>0)
	{
		if (!this.in->GetRandomPoint(result, num_tries)) return false; // sub-area empty? (note num-tries goes down)
		if (!this.ex->IsPointContained(result.x, result.y)) return true;
		--num_tries;
	}
	// Failed to find a point
	return false;
}

/** Constructor of sutraction area.
 @par in Shape that is included.
 @par ex Shape that is excluded.
 @return return a shape proplist representing a shape that includes the "in" shape and excludes the "ex" shape.
*/
public func Subtract(proplist in, proplist ex)
{
	// Create a new subtraction area
	return new BaseSubtraction { in = in, ex = ex };
}



/* Library initialization */

public func Definition(def)
{
	// Initialize function pointers in shape classes
	BaseShape = {};
	BaseRectangle = new BaseShape
	{
		IsPointContained = Shape.BaseRectangle_IsPointContained,
		GetBoundingRectangle = Shape.BaseRectangle_GetBoundingRectangle,
		GetRandomPoint = Shape.BaseRectangle_GetRandomPoint,
		Find_In = Shape.BaseRectangle_Find_In,
		Find_At = Shape.BaseRectangle_Find_At,
	};
	BaseCircle = new BaseShape
	{
		IsPointContained = Shape.BaseCircle_IsPointContained,
		GetBoundingRectangle = Shape.BaseCircle_GetBoundingRectangle,
		GetRandomPoint = Shape.BaseCircle_GetRandomPoint,
	};
	BaseIntersection = new BaseShape
	{
		IsPointContained = Shape.BaseIntersection_IsPointContained,
		GetBoundingRectangle = Shape.BaseIntersection_GetBoundingRectangle,
		GetRandomPoint = Shape.BaseIntersection_GetRandomPoint,
	};
	BaseCombination = new BaseShape
	{
		IsPointContained = Shape.BaseCombination_IsPointContained,
		GetBoundingRectangle = Shape.BaseCombination_GetBoundingRectangle,
		GetRandomPoint = Shape.BaseCombination_GetRandomPoint,
	};
	BaseSubtraction = new BaseShape
	{
		IsPointContained = Shape.BaseSubtraction_IsPointContained,
		GetBoundingRectangle = Shape.BaseSubtraction_GetBoundingRectangle,
		GetRandomPoint = Shape.BaseSubtraction_GetRandomPoint,
	};
	// shape class identity
	BaseRectangle.shape = BaseRectangle;
	BaseCircle.shape = BaseCircle;
	BaseIntersection.shape = BaseIntersection;
	BaseCombination.shape = BaseCombination;
	BaseSubtraction.shape = BaseSubtraction;
	return true;
}

/** Full landscape rectangle
 @return Return a shape proplist representing the rectangle covering the whole current landscape.
*/
public func LandscapeRectangle()
{
	return Rectangle(0, 0, LandscapeWidth(), LandscapeHeight());
}


/** Constructor of rectangle area. (x,y) is included; (x+w,y+h) is excluded. Automatically flips rectangles of negative size in any dimension.
 @par x Global left side of rectangle
 @par y Global top side of rectangle
 @par w Rectangle width
 @par h Rectangle height
 @return return a shape proplist representing a rectangle
*/
global func Rectangle(int x2, int y2, int w2, int h2)
{
	// normalize
	if(w2 < 0)
	{
		x2 += w2;
		w2 = -w2;
	}
	if(h2 < 0)
	{
		y2 += h2;
		h2 = - h2;
	}
	return new Shape.BaseRectangle {x = x2, y = y2, w = w2, h = h2};
}
