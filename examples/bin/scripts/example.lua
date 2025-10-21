-- init.lua
print("Hey")

local r1 = PopLib.Rect.new(0, 0, 10, 10)
local r2 = PopLib.Rect.new(5, 5, 10, 10)

print("r1 intersects r2:", r1:Intersects(r2))
print("intersection:", r1:Intersection(r2):__tostring())
