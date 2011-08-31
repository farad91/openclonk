/*-- 
	Down the Fountain
	Author: Mimmo_O
	
	An arena like last man standing round for up two to three players.
--*/


protected func Initialize()
{
	// Goal.
	CreateObject(Goal_LastManStanding, 0, 0, NO_OWNER);
	CreateObject(Rule_KillLogs);
	
	// Chests with weapons.
	CreateObject(Chest, 108, 230, NO_OWNER);
	AddEffect("IntFillChests", nil, 100, 3 * 36, this);

	// Objects fade after 5 seconds.
	CreateObject(Rule_ObjectFade)->DoFadeTime(7 * 36);

	// Smooth brick edges.
	PlaceEdges();
	
	//Water needs to be OK
	AddEffect("Refiller",0,100,6);
	return;
}

global func FxRefillerTimer(object pTarget, effect, int timer)
{
	for(var i=0; i<10; i++) if(!GBackLiquid(100,315)) InsertMaterial(Material("Water"),135,385);
}

global func PlaceEdges()
{
	var x=[268, 61, 149, 125, 133, 141, 53, 45, 29, 260, 276, 212, 197, 212];
	var y=[300, 308, 276, 252, 260, 268, 300, 292, 284, 308, 292, 197, 181, 316];
	var d=[1, nil, nil, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 1];
	for (var i = 0; i < GetLength(x); i++)
	{
		var edge=CreateObject(BrickEdge, x[i], y[i] + 5, NO_OWNER);
		edge->Initialize();
		edge->SetP(d[i]);
		edge->SetPosition(x[i],y[i]);
		edge->PermaEdge();
	}
	return 1;
}

// Gamecall from LastManStanding goal, on respawning.
protected func OnPlayerRelaunch(int plr)
{
	var clonk = GetCrew(plr);
	var relaunch = CreateObject(RelaunchContainer, LandscapeWidth() / 2, LandscapeHeight() / 2, clonk->GetOwner());
	relaunch->StartRelaunch(clonk);
	SetFoW(false,plr);
	return;
}


// Refill/fill chests.
global func FxIntFillChestsStart(object target, effect, int temporary)
{
	if(temporary) return 1;
	var chests = FindObjects(Find_ID(Chest));
	var w_list = [Bow, Musket, Shield, Sword, Club, Javelin, Bow, Musket, Shield, Sword, Club, Javelin, DynamiteBox];
	
	for(var chest in chests)
		for(var i=0; i<4; ++i)
			chest->CreateChestContents(w_list[Random(GetLength(w_list))]);
	return 1;
}

global func FxIntFillChestsTimer()
{
	SetTemperature(100); 
	var chest = FindObject(Find_ID(Chest));
	var w_list = [Dynamite, Rock, Dynamite, Firestone, Firestone, Bow, Musket, Sword, Javelin];
	
	if (chest->ContentsCount() < 5 || !Random((chest->ContentsCount())+4))
		chest->CreateChestContents(w_list[Random(GetLength(w_list))]);
	return 1;
}

global func CreateChestContents(id obj_id)
{
	if (!this)
		return;
	var obj = CreateObject(obj_id);
	if (obj_id == Bow)
		obj->CreateContents(Arrow);
	if (obj_id == Musket)
		obj->CreateContents(LeadShot);
	obj->Enter(this);
	return;
}

// GameCall from RelaunchContainer.
func OnClonkLeftRelaunch(object clonk)
{
	clonk->SetPosition(RandomX(120, 160), -20);
	clonk->Fling(0,5);
}

func KillsToRelaunch() { return 0; }
func RelaunchWeaponList() { return [Bow, Shield, Sword, Firestone, Dynamite, Javelin, Musket]; }
