/*-- Another Deciduous Tree --*/

#include Library_Plant
#include Library_Tree

private func SeedChance() { return 500; }
private func SeedArea() { return 400; }
private func SeedAmount() { return 10; }

local lib_tree_burned = Deco_Tree_Deciduous2_Burned;

public func GetTreetopPosition(pos)
{
	return Shape->Rectangle(-28,-20, 46,40)->GetRandomPoint(pos);
}

local Name = "$Name$";
local Touchable = 0;
local BlastIncinerate = 2;
local ContactIncinerate = 6;
local NoBurnDecay = 1;
local Components = {Wood = 5};