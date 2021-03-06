/*-- Kill logs --*/

func Initialize()
{
	// Only one at a time.
	if (ObjectCount(Find_ID(GetID())) > 1) 
		return RemoveObject();
}

public func Activate(int plr)
{
	MessageWindow(GetProperty("Description"), plr);
	return true;
}

func OnClonkDeath(object clonk, int killed_by)
{
	var plr = clonk->GetOwner();
	// Only log for existing players and clonks.
	if (plr == NO_OWNER || !GetPlayerName(plr) || !clonk) 
		return;
	ScheduleCall(this, "OnClonkDeathEx", 1, 0, clonk, plr, killed_by);
	return _inherited(clonk, killed_by, ...);
}

// parameters: clonk, owner, killed_by
global func GetAdditionalPlayerRelaunchString(){return _inherited(...);} // dummy

func OnClonkDeathEx(object clonk, int plr, int killed_by)
{
	if(!GetPlayerName(plr)) return;
	var name="Clonk";
	if(clonk) name=clonk.Prototype->GetName();
	// Assert there are three StringTbl entries for each.
	var which_one = Random(3) + 1;
	var log="";
	if (!GetPlayerName(killed_by))
 		 log=Format(Translate(Format("KilledByGaya%d", which_one)), GetTaggedPlayerName(plr), name);
 	else if (plr == killed_by)
		log=Format(Translate(Format("Selfkill%d", which_one)), GetTaggedPlayerName(plr), name);
 	else if (!Hostile(plr,killed_by))
  		log=Format(Translate(Format("Teamkill%d", which_one)), GetTaggedPlayerName(plr), name, GetTaggedPlayerName(killed_by));
	else log=Format(Translate(Format("KilledByPlayer%d", which_one)), GetTaggedPlayerName(plr), name, GetTaggedPlayerName(killed_by));
	
	// okay, why is GetRelaunchCount not a global function..?
	// and why are the relaunches not stored in a static variable or a singleton that can be accessed somehow..
	var relaunches=nil;
	for(var goal in FindObjects(Find_Or(Find_Category(C4D_Goal), Find_Category(C4D_Rule))))
	{
		relaunches = goal->~GetRelaunchCount(plr);
		if(relaunches != nil) break;
	}
	if(relaunches == nil) relaunches=GameCall("GetRelaunchCount", plr);

	if(relaunches != nil)
	{
		var msg="";
		if (relaunches < 0) // Player eliminated.
		msg = Format("$MsgFail$", GetTaggedPlayerName(plr));
	else if (relaunches == 0) // Last relaunch.
		msg = Format("$MsgRelaunch0$", GetTaggedPlayerName(plr));
	else if (relaunches == 1) // One relaunch remaining.
		msg = Format("$MsgRelaunch1$", GetTaggedPlayerName(plr));
	else // Multiple relaunches remaining.
		msg = Format("$MsgRelaunchX$", GetTaggedPlayerName(plr), relaunches);
		
		log=Format("%s %s", log, msg);
	}
	
	// this is also not a global function, but that is okay. So..
	// get additional strings from goals/rules ("%s is now king!!")
	for(var goal in FindObjects(Find_Or(Find_Category(C4D_Goal), Find_Category(C4D_Rule))))
	{
		var other=goal->~GetAdditionalPlayerRelaunchString(clonk, plr, killed_by);GameCallEx("GetAdditionalPlayerRelaunchString", clonk, plr, killed_by);
		if(other)
		{
			log=Format("%s %s", log, other);
		}
	}
	
	// get additional stuff from scenario
	var other=GameCall("GetAdditionalPlayerRelaunchString", clonk, plr, killed_by);
	if(other)
	{
		log=Format("%s %s", log, other);
	}
	
	// also allow global callback function to add to death messages
	other = GetAdditionalPlayerRelaunchString(clonk, plr, killed_by);
	if(other)
		log = Format("%s, %s", log, other);
		
	Log(log);
}

local Name = "$Name$";
local Description = "$Description$";
