for i=0,map.Width do
	map.Raise(i, 0)
end
for i=1,map.Height do
	map.Raise(0, i)
	map.Raise(map.Width, i)
end
map.Raise(1, 1)

map.SetTile(2, 4, 1, -1) --single sand tile
map.SetTile(2, 0, 2, -1) --single stone tile on cliff

function hasSpace(x, y)
	for sx=x-1,x+1 do
		for sy=y-1,y+1 do
			if map.Elevation(x, y) ~= 0 then
				return false
			end
		end
	end
	return true
end

for i=0,10 do
	x = math.random(5, map.Width - 5)
	y = math.random(5, map.Height - 10)
	if hasSpace(x, y) then
		map.Raise(x-1, y-1)
		map.Raise(x  , y-1)
		map.Raise(x+1, y-1)
		map.Raise(x-1, y  )
		map.Raise(x  , y  )
		map.Raise(x+1, y  )
		map.Raise(x-1, y+1)
		map.Raise(x  , y+1)
		map.Raise(x+1, y+1)

		-- Raise it again for the middle bit.
		map.Raise(x  , y  )
	end
end
