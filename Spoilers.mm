
.ND "December 10, 1987"
.TL
(Net)Hack ``Spoilers'' Reference Manual
.AF "{amd, ihnp4, microsoft, ucscc}!sco!stevem"
.AU "Edited by Stephen Marino*"
.AS x .5i
.\" This is a kludge to get the date on the cover sheet.
(Net)Hack is a Rogue-like dungeon game that runs on a variety
of computer systems in as many implementations.  Although much
of the information in this document is generic to Hack, it is
slanted toward the newest release, NetHack.  This reference is
neither a man page nor a basic instruction manual. Beginners
and those who wish to learn strictly from experience should
refer to the \fIGuide to the Mazes of Menace\fR, a revised
edition of Ken Arnold's \fIGuide to the Dungeons of Doom\fR
by Eric S. Raymond.  The (Net)Hack ``Spoilers'' Reference Manual
is an exhaustive compilation of facts,
strategies, and what are known in the vernacular as SPOILERS.
The reader is warned that this guide leaves little to the
imagination.  Although probably a bit overwhelming for the
complete novice, this manual should prove useful
to those who want to know almost everything about the game. 
.SP 2
\*(DT
.\" These are footnotes on the cover sheet.
.FS *
Many people are responsible for the text of this document, but it
is principally the work of Dave Harmon, Greg Samson, Melissa Silvestre,
Robert Thau, Tony Lazar, ``Daniel G. Winkowski'', Carlton Hommel and
``Colonel'' George L. Sicherman, who all created a set of hint files
for Hack 1.0.1.  Also, my thanks to Douglas Rosengard, whose tutorial
taught me a couple of things even I didn't know.
My apologies to anyone I've missed.
.FE
.AE
.MT 4
.CS
.nr % 0
.bp
.ds +3 +2 +0 +0
.PH ''SPOILERS''
.PF 'DRAFT'%'12/10/87'
.ce 3
\s+2\fB(Net)Hack ``Spoilers'' Reference Manual\fR
.sp
\fIEdited by Stephen Marino\fR\s-2
.sp
\fB{amd, ihnp4, microsoft, ucscc}!sco!stevem\fR
.sp
.H 1 "INTRODUCTION"
If you are relatively new to Hack, keep the handy reference sheet
in Appendix A close by.  Beware of features that may not apply to
your version; NetHack features are flagged where possible.
.P
Your character begins on level one of a 30+ level dungeon.
Accompanying you is your faithful dog, which should be named (with the
``C'' command).  The idea is to kill monsters, gather gold,
and (eventually) retrieve the Amulet of Yendor.  The primary objective, however,
is to survive, which isn't that easy.  To help (or hinder) your progress are
magic items, traps and other contrivances developed by scores of programmers,
all calculated to drive the player mad with frustration.
'
.H 2 "How to Use this Guide"
'
This paper is organized into the following sections:
.BL
.LI
Basic Instructions
.LI
Strategy
.LI
Rooms and Other Bad Places
.LI
To Eat, or Not to Eat
.LI
Monsters
.LI
Weapons and Armor
.LI
Scrolls, Potions, Rings and Wands
.LI
Extended Command Set
.LI
Special Items
.LI
Tricks and Other Non-intuitive Actions
.LE
.P
The basic structure is that of a reference manual, with basic and
strategy tutorials to the front.  The Table of Contents should prove
useful for quick indexing by subject.
.H 1 "BASIC INSTRUCTIONS"
'
.H 2 "Getting Started"
'
There are some basic commands that you
should be familiar with;
refer to the command chart in Appendix A
or the ``?'' command.  Learn all the move keys \- diagonal movement can
be critical.
This section discusses the most basic considerations of playing the
game.  The first decision to be made is the choice of character to
do your dirty work.
'
.H 2 "Character Classes"
'
The table below lists the various character classes, their equipment
and beginning stats.
.TS
tab (#),center,expand,box;
c c c c c c
l | l | n n n | l.
Class or Name#Special Items#St#Ac#Hp#Other	
_
Archaeologist\(de#pickaxe & icebox#14#7#12
Cave(wo)man\(de#+1 club#18#8#16
Elf# #16#9#16#sword
Fighter\(de# #17#7#14#two-handed sword
Healer#stethoscope#15#10#16
Knight\(de#suit of armor#10#3#12#long sword
Ninja# #10#7#15#katana, shurikens
Priest(ess)#+1 mace, blessed#15#4#13#2 spellbooks
Samurai# #16#4#16#katana
Tourist\(de#camera (w/flash)#11#10#10#+2 darts, lots of food
Valkyrie#+1 long sword#17#6#16#+3 shield
Wizard\(de# #16#9#15#lots of magic items
.TE
.BL
.LI "\(de"
Denotes standard Hack character classes.  All others exclusive to
Nethack.  ``Speleologist'' has been renamed ``Archaeologist'' in Nethack.
Beginning stats shown are also for NetHack.
.LE
.P
Each character has advantages, but some are more unusual than others.
The Tourist is a weak human being, but is equipped with a
a camera and inexhaustible flash unit that blinds monsters.  The 
Healer (with a stethoscope to diagnose monsters) and the Tourist
are both weak and difficult to keep alive. (The idea
is to get by until weapons, armor and magic can be found, so that
the benefits of the peculiar items can be enjoyed.)
The Archaeologist/Speleologist starts with a pickaxe
to dig and box to keep food.
The remaining characters are self-explanatory.
'
.H 2 "Words to the Wise"
'
Consider the key points that are listed in this section; they may save
you time and frustration.
.H 3 "Choose an appropriate character."
To start out, choose a character that can survive a few beatings while
you learn your way around; the archaeologist/speleologist, healer,
tourist and wizard are virtually unarmed and defenseless.  
.P
.H 3 "Your best friend is your dog."
Be patient with your dog; it can save your life.
You will notice that he gets in your way and sometimes eats what you
would like to get hold of.  He eats what he needs to, but he leaves
food rations alone unless you throw one to him.  If the message is
displayed: ``You feel worried
about the dog,'' it means that he is starving.  Throw him a tripe ration,
kill something, or (if you can spare it) toss a food ration.
The message ``You have a sad feeling for a moment, then it passes,''
means that your dog has died.
Many people prefer to leave their dog behind on the first level.
Considering that your canine is a match for almost any creature in the
dungeon, you are well-advised to keep it around.  A leash
allows you to keep the animal with you at all times, but also tends
to make him get in your way more often.
.P
.H 3 "Don't let the monster get the upper hand."
When moving around or running from monsters, the idea is to give them
as few chances as possible to hit you.  If you end a move right
next to a monster, it will have first crack at you.  Some monsters, like
leprechauns and bats, can leap an extra space and still hit you.
When moving in a room with monsters in it, plan your moves ahead.  Remember
that you can enter a door only at right angles to the wall, but you can
attack to or from a door diagonally.  This applies to monsters too.
.H 3 "Beware Swarms of Orcs."
It is a good idea to use the ``c'' (call) command to name your
weapon ``Orcrist.''  This will give you a hitting bonus against the
swarms of orcs you will encounter.  Orcs are very common, and if you
run into one, the others aren't far off.
.H 3 "Where possible, eat what you kill."
You must eat food soon after the display indicates you are hungry.
Dead monsters are a necessary supplement to your diet.  Some characters
start off with little or no food.  Not only do corpses ward off
starvation, but some monsters will give you
special powers.  However, you must be very careful to only eat monsters
killed within the last few turns and eat only those that are safe for
consumption.  (Refer to the section on Food for more details.)
.P
.H 3 "In Hack, relief is spelled E-l-b-e-r-e-t-h."
One of the most vital tactics in Hack is the use of the E)ngrave command
in conjunction with the magic word ``Elbereth.''  When written on the
floor of the dungeon, this word represents a sort of truce with monsters.
When surrounded
or close to death, you can ``go under Elbereth,'' as the expression goes,
and monsters will not attack again until you strike at them first.
(There are several exceptions;
see the section on Elbereth under ``Tricks and Other Non-intuitive
Actions'' for detailed instructions.)
.H 2 "Learning the Ways of Magic"
At low levels, you only have to worry about 3 things: your hit points, your
food supply, and dealing with cursed magic that can temporarily cripple you.
Two particular items that require great consideration are a wand of
wishing and a scroll of genocide.  These items should not be wasted;
give some thought to possible uses before you find one.  There is more
than enough information in this document to aid your decision.
.P
.H 3 "Test potions and scrolls carefully."
While playing, it is best to read unknown scrolls and try potions
before you clear an entire level; scrolls and potions of detection
will yield ``strange feelings'' if there is nothing to detect.
(You might drop some gold, a magic item, a food item,
and ensure that a monster is around before testing to guarantee results.)
It is also possible to fling potions at monsters and observe their effects
(see ``Potions'').
.P
It is not a good idea to test magic when your hit points are really low;
scrolls of fire, create monster, and potions of sickness can kill you.
With most magic items, after testing you will be 
able to identify them the next time you pick one up, or at least have an idea 
what they are.  If you are prompted to name an object you've just used
(usually scrolls) this is the name it will be given if you find it again.
This document should enable you to recognize these items quickly.
.P
In the case of scrolls, there are three whose action is sufficiently
nasty that it is important to take precautions until they have been
discovered: Damage Weapon, Destroy Armor and Amnesia.  The respective
dangers are that your weapon will be ruined, your armor destroyed and
your spells forgotten.  One way to prevent this wanton destruction of
your property is to (prior to reading scrolls) wield an alternate weapon,
wear an alternate armor or something over your primary armor, and don't
transcribe spells until you've read the Scroll of Amnesia (``Who was that
Maude person anyway?'').
.P
.H 3 "Zap wands immediately."
If they are non-directional you will usually 
find out what it is immediately.  A non-directional wand that doesn't seem to 
do anything is a wand of secret door detection.  You may want to conduct such 
tests in a room where you suspect a secret door, but haven't searched it out 
yet.  If it asks for a 
direction, hit <space> to cancel, and next time you have a cooperative monster,
engrave Elbereth and zap it at the monster in such a way that if it's a
``bolt'' wand it won't bounce back at you.  (Stand at a diagonal from the
monster.)
Don't accidentally hit your dog!  
.P
.H 3 "Never experiment with rings."
Use identify scrolls on rings.  Be particularly careful with rings (and
weapons and armor for that matter) from a ghost hoard, which are
\fIstickeycursed\fR, which means that they are not necessarily cursed
in the usual sense (with negative enchantment) but they cannot be removed
without a remove curse.  If, for example, you put on a stickeycursed
ring of levitation, you won't be able to pick up anything, including
a scroll of remove curse.
.H 1 "STRATEGY"
.H 2 "Upper Levels"
.H 3 "Let the dog fight for you when you are badly hurt or outclassed."
He is bigger and stronger than you.  Don't steal all his kills;
he can starve to death. He will eat more than he needs, but the more he eats,
the faster he grows into a big dog.  There is no monster except a cockatrice
that a big dog cannot take on with a reasonable chance of survival.  
.P
His body is one guaranteed non-hostile food source, provided you don't 
wait so long you die before you can kill him.  (Due to the ``You avoid
hitting your dog feature,'' the only way to kill your dog is with
a wand or projectile weapon, or hitting him while blind.)  Starvation excepted, 
however, (and not then if you have an alternative) DO NOT kill your dog 
out-of-hand! He may get in the way, but he is a powerful ally.  Also, the
minute you eat a dog, every monster on the level will head for you with blood
in its eye(s).  There are a few monsters he will not normally attack, such as 
shopkeepers, and others that he won't attack until he sees you fighting 
them.  On the other hand, he will immediately attack bats ond orcs.
A ring of conflict will make him fight anything.
Don't test wands out on him; your canine is not invulnerable.  
.P
If you meet a wild dog, the best bet is to throw food at him \- anything 
you could eat.  Tripe 
is wonderful, but don't bother throwing rotted corpses.
If you find a ``wild'' dog above 8th level or so, or a named monster 
anywhere, you are on a ghost level.  Watch out for the dog's former owner.
Please name your dog, even if it's Ralph. That way, other people will have the 
same advantage if you die and become a ghost.
.P
Don't try to maintain more than two tame dogs \- if you get more than 
that, abandon them (save some tripe in case you need to go back to the level)
or, if you are powerful enough to face enraged hordes, kill and eat the extras 
next time you're hungry.  Scrolls of 
taming produce tame monsters that should be treated as polymorphed 
dogs.  Tame monsters all eat a lot, except the cockatrice, and you can't eat 
anything he kills.
(By the way, if you get a tame cockatrice, DO NOT GO AROUND BAREHANDED!
If you are not wearing gloves, wield anything, even an egg! Otherwise, 
you run a terrible risk of bumping into it with your bare hands and
becoming petrified.)
.P
.H 3 "Don't get careless."
A major cause of upper-level fatalities, or
fatalities at any level, is not keeping an eye on your hit points.  If you are
low on hit points, freeze with your hands off the keyboard and figure out what
to do.  Can you get away from the monster?  Do you have a secret weapon?
The first thing to do is often to write Elbereth.  (Magic Markers can be used
to create an indestructable Elbereth that works the same as a scare monster
scroll; see Magic Marker under ``Tools.'')  Even a puny monster can
kill you if you don't watch your hit points.  Monsters to keep an eye on at
low levels are hobgoblins and acid blobs.  If you have 8 or fewer HP,
a hobgoblin can kill with one blow.
Getting killed by acid blobs is always embarrassing,
because the only way they can hurt you is if you hit them.  Don't chase acid 
blobs if you're low on HP.
.P
.H 3 "You may want to eat a leprechaun and a floating eye."
The first is a matter of preference, since the ability to teleport
randomly can be a blessing and a curse.
The latter, your dog will be happy to kill for you,
but you will, as usual when he
kills monsters, have to move fast to prevent him from happily eating it for
you.  You should be able to kill a leprechaun by 3rd level or so.
in NetHack, wizards gain teleport control at experience level 5, so it
is a good idea to eat one.  (Other character classes gain the same
ability at experience level 10.)
You can use a wand to kill a leprechaun,
but if planning an extended visit in the dungeon, just wear him
down hand-to hand, perhaps softening him up with a couple of throwing weapons.
To keep him from stealing all your gold, drop it in a corridor, and once you
disturb him, defend the door to that corridor. (Remember, he's much
faster than you are).  If he gets your gold anyway, he and the
gold are still somewhere on the level.  Again, the dog may be some help
in the fight, but you will have to grab the corpse before he does.
Another strategy is to start with dog well away from both you and leprechaun.
Throw something at leprechaun from 3 squares or so away, and go under Elbereth.
He will come up to you, and your dog may fight 
him and kill him at your feet.  This works best with you standing in a door 
and your gold behind you in a corridor, and your dog wandering someplace on 
the far side of the room.
.P
.H 3 "Try to conserve your missiles and magic."
You can just take it slow,
and deal with monsters as leisurely as your food supply (including their
contributions) permits.  If you conserve firepower early, then you can use it 
against tougher monsters when you really need it.  On the 
other hand, you might find that your weapons and skill have improved to the 
point where you can deal with tougher monsters.
Remember, if you use a potion of extra-healing to gain
4 HP now, you may not have it when you're down to 10 out of 40 HP.
A partial exception is wands of wishing.  
You need one good weapon, one rustproof armor (or an elven cloak to protect
your armor),
miscellaneous such as 3 tins (usually spinach), or a powerful magic item.
This applies to wizards, of course \- other types should wish for whatever they 
are short of.  Don't wish for scrolls of genocide just yet, either.  If you
are genuinely doing all right, you might save a wish for future needs. 
.P
In NetHack, the presence of spellbooks makes your choices somewhat different.
(Refer to the section on Spellbooks in the ``Extended Command Set'' section.
.P
.H 3 "Do anything to survive."
If you are starving and no edible monsters
in sight, then kill your dog!  If you are fainting, down to 2 HP, and cornered
by a hobgoblin, then zap anything you can, try all your rings, quaff all your 
potions, read all your scrolls.  You might be teleported, or discover a ring
of regeneration or a potion of extra healing, or a wand of fire.  If you
are killed by the rebound of your magic missile, at least you tried.
'
.H 2 "Lower Levels"
'
.H 3 "Be prepared for crowded rooms."
Below level 8, Throne Rooms, Treasure Zoos and Beehives become regular
hazards.  If you are reasonably equipped, you can back into the corridor
and fight the monsters one at a time.  If you aren't, it's best to teleport,
drop a scroll of scare monster, or find some way of retreating to a staircase.
Bolt wands and spells can be counted on to kill more than one monster at
a time.
Don't count on Elbereth to save you when you are badly outnumbered.
If you've found a ring of conflict, you are well-equipped to
handle the situation, as the monsters will fight amongst themselves.
Don't wear it constantly; it is only
useful when there are two or more monsters near you and the ring consumes
a great deal of energy. 
In NetHack, a spell of Charm Monster can be used to turn several monsters
to your side; they will behave like dogs and kill your enemies.
.H 3 "Rob stores; the risk is worth it."
You should be robbing stores according to the procedures described in
the section on ``Shops.''  Stealing may seem unethical, but 50 types of
monsters bent on killing you gives you a sense of perspective.
.H 3 "Don't throw away bad potions."
Throw them at monsters.  A potion of sickness can do considerable damage.
You can also dip your weapon in them.  Save potions of confusion.  (See 
``Potions.'')
.H 3 "Genocide with consideration."
If you have scrolls (or spell) of genocide, it is best to eliminate
xans or, if you're short on missiles, cockatrices.  Don't genocide dragons;
you'll need to eat one if you don't find a ring of fire resistance.
.H 3 "There are eight truly awful monsters in the dungeon; know them."
In order of fearsomeness: Humans (shopkeepers), Dragons, Demons, Ettins, Eels,
Cockatrices, Xans, and in NetHack, Nymphs. 
.VL 12
.LI Humans
When angered, nothing lowers your hit points quite like a shopkeeper.
.LI Dragons
Their flames are fearsome and have been known to travel through walls
and past boulders.  Even with a ring of fire resistance, beware of
boiling potions that have negative effects.  Kill them quickly or run
away fast.
.LI Demons
Demons get multiple hits and tend to replicate.  A wand or spell of
turn undead works well.  Blessed weapons also fare well.  Use a wand
of cancellation to prevent replication.  Demons usually appear when
you are fooling with fountains.
.LI Ettins
They hit hard and fast, doing about the same damage as a shopkeeper.
.LI Eels
Invisible, eels hide in muddy pools (}).  When you walk into a room
with pools, a message is displayed ``It looks rather muddy in here.''
If you aren't careful, one will quickly pull you in to drown.  If you
have the ability to teleport at will, you will do so automatically.
.LI
Cockatrices
Their touch turns you to stone instantly, their hissing may do the
same, only more slowly.  Dead lizards are useful against them.  The
problem with cockatrices is that, once dead, are still a hazard.
Don't forget where you left the corpse and don't inadvertantly pick
it up.
.LI Xans
Their pricks eventually render you unable to move, collapsed.  Time
heals you slowly, but royal jelly (found in hives) restores movement
instantly.  Levitation can be used to avoid xans; they are short
creatures. 
.LI Nymphs
They are much nastier in NetHack than in other games.  Although usually
asleep, they can wander and hit so fast that, if you have no handy
killer magic, they can rob you blind.  They teleport instantaneously
unless you use a wand of cancellation.  (Think of what could happen to
you if, in two passes, a nymph has stolen your weapon and your armor.)
.LE
.H 3 "To win the game, you must go to hell and back."
Your principal goal, the Amulet of Yendor is found in the center of Hell,
guarded by the Wizard of Yendor and his dog, surrounded by a moat.
Once below level 26, you will notice there are no stairs going down.
To get to hell, you must read a teleport scroll while confused; you will be
asked "What level?"  Enter ``40'' and to hell you will go.
You will burn to a crisp without fire resistance.
You must levitate over the moat or blast the waters with fire and dig
through the wall to reach the Wizard.
(If you genocide dogs, the Hell Hound will also be destroyed.)
.H 1 "ROOMS AND OTHER BAD PLACES"
Rooms can be empty, filled with traps, treacherous pools with eels,
or even a beehive.  The types of rooms listed in this section are
much easier to deal with if you have a ring of conflict to force the
denizens to fight amongst themselves.
'
.H 2 "Assorted Shops"
'
Shops add much of the spice to hack.  Shops
are rooms filled with goodies guarded by a shopkeeper. Shops
may specialize in a particular object (armor, weapons, scrolls, wands, rings) or
they may be general (antiques).  There are three things you can do in a shop: 
buy things, sell things, or steal.   
.P
Buying things is simple.  You pick up items and p)ay your shopping bill.  
(Note: prices for magic items are high, but you often may want to buy food or
a weapon if you aren't equipped to steal.)
.P
Selling things is almost as simple.  You drop (not throw) the items, and if 
the shopkeeper is interested, he will buy it from you.  WARNING: if not 
interested, he may take the item without paying (i.e. the keeper of a
book shop isn't interrested in your mace or potion).
.P
Stealing is harder.  You must kill or sleep the shopkeeper or get out past 
him.  A nasty but lucrative tactic is to enter a shop, sell everything you can 
to him, including extra rings, cursed items, and/or dead monsters, then take 
(not buy) back everything you want to keep, anything of his you can carry, and 
steal it all, including the gold he paid you.  You will 
have to rely on your brains, not your brawn.  Note that the shopkeeper usually 
has some treasure of his own that may not appear until he is dead.
.P
.H 3 "Strategies for dealing with shopkeepers" 
.BL
.LI
Teleport out: keystone kops will immediately pursue you; the shopkeeper
will follow somewhat after, asking "Did you forget to pay?"  Beware the
pies thown by the keystone kops;
the shopkeeper will get angry if he gets hit and take it out on you.
.LI
Dig your way out.  
.LI
Teleport the shopkeeper away with a wand of teleportation; just
don't meet him on the way out.
.LI
Avoid the shopkeeper: lure the shopkeeper away from the door, then 
zap him with a wand of sleep.  Tiptoe around him and make good your
escape.
.LI
Kill the shopkeeper: you better have a wand!  Killing humans is very bad
luck.  On the run, these retired barbarians will
wipe up the floor with you if they catch you.  They are faster than you and
\fBthey ignore Elbereth\fR. If you quaff a potion of invisibility, they will stand
in front of the door no matter what you shoot or throw.  The shopkeeper
knows who's attacking him, and if you get too close, he'll get you.
.LI
Train your dog: this isn't as hard as it sounds. It can be fun if you're not a
taker of risks.  Get your dog inside the shop and sit in the doorway until
the dog puts an object directly in front of you.  You can step forward and
take the item without being charged for it.
When he moves away, toss him a tripe ration.  Having been rewarded,
your dog will will bring other objects to you in the same manner. 
(Note: the shopkeeper may stand
on this critical spot if the dog has displaced him; just be patient.)
.LI
If by some chance you have a shop with a bunch of orcs in the back,
Elbereth one step behind the door, and kill the orcs as they exit the room.
(You may have to take a step into the room or throw something to wake up the
orcs).  The orcs will carry many items outside the shop for you, and you will
not be charged for them. The shopkeeper may not even declare his shop 
pillaged.
.LE
.P
Shopkeepers have long memories.  They will refuse to let you out
of a shop if you have stolen any items from it, even if you are not trying
to steal anything on this visit.   Shopkeepers are also crafty enough to
stay out of your line of fire, unless you are within 2 spaces of the door. 
Make sure you have a means of escape before trying anything, since you will
have to pay for or steal anything you use up or get stickeycursed by!  If you 
know which scroll is teleportation, then you can use one of his, but if trying
his scrolls out to find a teleport scroll (don't even try it except in a book
store) you run a risk of being stranded.  Trying wands out in a walking-cane 
shop will avoid that problem (since wands are not used up in one shot) but
if any wand is fired at, or any ray hits (look out for rebounds) the
shopkeeper, he will get mad!  If you really wipe out a shop, the storekeeper
will declare it pillaged, and instead of buying, he will accept contributions.
.P
.H 2 "The Magical Memory Vault"
This little 2x2 room holds the current
balance of the Magical Memory Bank.  It has no doors connecting it to any
room or corridor on the level.  You may locate it in three ways. 1)
read a scroll of gold detection.  The four piles of gold are unmistakable.
2) read a scroll of magic mapping.  Any 2x2 rooms on the level not
connected to any other rooms will be a Magic Memory vault.  3) stumble
upon one.
.P
There are three ways of getting to a Magical Memory vault.
1) teleport in.  This is the fastest way in, but will you be able to get out
again? 2) use a wand of digging. 3) Find the phrase ``ad ae?ar um''
(Latin: \fIad aerarium\fR,``to the money.'') and search it until
you find a passage that contains a ``single-use'' teleport trap and use it.
.P
If you stay in a vault too long, a guard will come by and ask you
to identify yourself, and if he doesn't know you,
to drop all your gold.  If you have no way to teleport out or defeat
the guard, you lose both your money and the money you've stolen.
The key is to identify yourself as ``Croesus'' (as in ``Rich as Croesus.'').
The guard will then leave you to your looting.
.P
In some older versions of Hack (1.0.3) it is also possible to
deal with the guard this way:
.AL
.LI
Drop your gold, and move one step toward the guard (once he
gives you room).  He will move a step down the corridor.
.LI
Take a step back to pick up the gold; the guard will stay put.
.LI
Step toward the guard again, till you are next to him (Don't
hit him!).  Drop gold again.  He will advance again.  
.LI
Repeat until guard reaches end of corridor, where he will disappear.
.LE
'
.H 2 "David's Treasure Zoo"
'
The zoo corresponds to a monster room (sometimes know as ``party rooms'')
in Rogue.  There are some differences: A zoo is packed with monsters,
except for the row facing the door.  There are no magic items, (except for 
what materializes when you kill a monster) but a pile of gold is under
every monster.  Monsters in a zoo are best dealt with with a ring of 
conflict, but zapping a missile wand (fire, cold, magic missile, death,
etc.) will kill many monsters.  Otherwise, you better run as fast as 
you can away from the zoo and come back when you are better prepared.
Best thing is to use Elbereth and a strong wand.  If a
leprechaun is in the zoo,
when you use a wand or a ring of conflict, he will go around picking up the
gold left by dead monsters.  When he has all the gold, he will teleport, which
is wise of him, because he is carrying a lot of gold.
Orcs will also pick up gold but, or course, can't teleport.
'
.H 2 "Hives"
'
Lairs filled with killer bees.  Asleep at first, they will swarm if
you remain long.  Hives have several lumps of royal jelly, a substance
that simultaneously fills your stomach, heals and increases strength.
It also heals the effect of xan pricks.
'
.H 2 "The Morgue"
'
These are filled with ghosts with strange names.  When
you disturb their rest, they file out and attack.  Note that these
ghosts are not of former players.
'
.H 2 "Throne Rooms"
'
Like Treasure Zoos, Throne Rooms are packed with monsters.  In order to
get a crack at the throne, you must clear some of them.  A ring of
conflict is useful.  When you approach the throne, use the # sit command
if you dare.
Although the message ``you feel somehow out of place...'' is always displayed
when sitting down, there is a one in three chance that something will happen.
As with fountains, after a random number of operations, the throne disappears:
``The throne vanishes in a puff of logic.''
.P
.S 9 10
.TS H
tab (#), center, box;
c c
l l.
Message#Effect
_
.TH
``you feel suddenly weaker.''#Lose 1-2 strength, plus hp damage
``you feel suddenly stronger.''#Gain strength
``A massive charge of electricity shoots through your body!''#Electrocution, great damage
``you feel much, much better!''#Cure all
``you notice you have no gold!''#That's correct
``you feel your luck is changing.''#Adds to luck or grants a wish
``an image forms in your mind.''#Magic mapping
``your vision sharpens.''#See invisible
``you feel threatened.''#Aggravate monster
``you feel a wrenching sensation.''#Teleportation
``you are granted a gift of insight!''#Identify pack
``your mind turns into a pretzel!''#Confusion
_
``you hear a voice echo:''#[Three possible:]
``Your audience has been summoned, Sire!''#A nasty monster appears
``By your Imperious order Sire...''#Genocide
``A curse upon you for sitting upon this most holy throne!''#Blindness or curse pack items
.TE
.S
'
.H 2 "Mazes"
'
Mazes are found on the lowest levels of the
dungeon.  The exact level may vary depending on your version of hack.  
The maze takes up the entire level, and is inhabited by a variety of
strong monsters.  One peculiarity of the maze is that there are no 
staircases down.  All staircases in mazes lead up, even if you had just
ascended the staircase from a lower level.  The only way to get to the second
or greater maze levels is to fall several levels through a trapdoor.  Even 
then, you will have to repeat the fall if you want to return. You may wish to 
save a scroll of mapping for the maze, especially if you possess a ring of 
teleport control. Also useful are potions of object detection (see Amulet),
monster detection, and blindness (if you have telepathy).
Every maze is inhabited by a minotaur (m) who carries a wand of digging.
It is rumored that a wand of wishing can be found on a particular level.
'
.H 2 "Traps"
'
There are various types of traps, designed to injure, incapacitate,
or relocate you.
.VL 22
.LI "dart trap"
Small damage, rarely poisoned.
.LI "arrow trap"
Greater damage, rarely poisoned.
.LI "pit trap"
Small damage, temporary imprisonment. Greater damage if spikes are
present.
.LI "bear trap"
Little damage, temporary imprisonment.
.LI "sleeping gas trap"
Sleeping gas is dangerous during battle; you can die while asleep.
Lasts several turns.
.LI "teleport trap"
Send you to random spot on same level.
.LI "level teleport trap"
Sends you to a random lower level. Message reads: ``You
are momentarily blinded by a flash of light.'' It is quite possible to
teleport to level 30, where, if you do not have fire resistance or a
ring of fire resistance on your finger (in your pack does you no good!)
the message is displayed: ``You arrive at the center of the earth \-
Unfortunately that is where hell is located \- You burn to a crisp.''
.LI trapdoor
Drops you to the next level.
.LI "squeaky board"
Makes a sharp sound that alerts monsters to your presence.
.LI "anti-magic field"
Drains some of your magic energy.
.LI "water trap"
Squirts water that can rust your weapons and armor.
.LI "magic trap"
Has several effects, some which are familiar, but not all of which are
real.  The following is a list of messages associated with magic 
traps and their meanings.  The most common effect is a blinding flash
of light followed by the creation of one or more monsters around you.
.LE
.TS
expand, tab(#), center, box;
c c
l l.
Message#Effect
_
``You are momentarily blinded by a flash of light.''#Blindness; create monster
``You yearn for your distant homeland.''#No effect
``Your pack shakes violently!''#No effect
``A shiver runs up and down your spine.''#No effect
``You hear a distant howling.''#No effect
``You hear a deafening roar!''#Create monster
``You feel someone is helping you.''#Lift curse
``You feel charismatic.''#Tames monsters around you
``You feel in touch with the Universal Oneness.''#Lift curse (hallucinating)
``You feel like you need some help.''#Lift curse (hallucinating)
``A tower of flame bursts from the floor.''#Reduces hit point maximum
.TE
.P
You can still fight and use Elbereth from bear and pit traps.
All traps can be discovered by searching, or by passing near with a
ring of searching.  A wand of secret door detection may also find them.
'
.H 2 "Fountains"
'
You can drink (quaff) from fountains or dip things in them.  The water
may bestow certain benefits, but such instances are rare.  Most of the
time you will find ``The cool draught refreshes you.''  Sometimes a gem
will appear.  There is a small
chance of causing the fountain to overflow.  When this happens, stay clear
of the pools, which are displayed as ``}'' (rather than as fountains,
``{'').  If you step into one, you will certainly drown, unless you
have teleport ability. (The game automatically attempts teleportation
when drowning.)  It is also possible to unleash Demons; this
is likely to be very unlucky.  (See ``Monsters.'')
Dipping a long sword can be useful (see ``Excalibur'') but there may be
some initial damage and enchantment may not be achieved before the fountain
dries up.  The best procedure seems to be going under Elbereth before
dipping.  You may see several messages, as summarized here:
.P
.so fount.tbl
.H 1 "FOOD"
'
.H 2 "Hunger, Weakness and Death"
'
In Rogue, it is best not to eat until the message ``You feel weak'' is
displayed.  Not so with Hack (and especially NetHack).  Your strength
drops by one point when the ``weak'' message is displayed, and there
may be lasting effects.  Eat when you are hungry.  If you
eat two food rations in a row, or an enormous amount of other stuff, you will
become Satiated ('You're having a hard time getting all that food down').
\fBDo not eat while satiated\fR.  You may choke and die.
'
.H 2 "Monsters"
'
Eating fresh-killed monster corpses is good for saving your 
rations.  However, some are poisonous, and all rot after a short time.  
Eating some monster corpses will give special abilities or effects (see 
the table at the end of this section),
but may simultaneously do poison damage.  Eating rotted (tainted) 
corpses is lethal within 20 turns at most, unless you have a handy healing or 
extra-healing potion. If you do, drink it fast. Poison resistance will not
protect against rotted corpses.
'
.H 2 "Food Items"
'
Aside from monsters, food seems to keep forever.  Most 
named items have little food value.  Food rations however, are a super
food which will take you from fainting to healthy in one shot.  Some pancakes
also seem to do this.  Fortune cookies contain hints from a rumor file.
Ignore ``clues'' to next level; they are random.  Hints about monsters, methods,
etc. are usually good.  Tripe is dog food.  If you eat it, it will usually
behave as a food ration, but you may throw up, and be left only slightly
better (or worse) than you started.  Small food items can at least delay
starvation. Carrots will cure blindness, and a clove of garlic can be
wielded against a vampire.  A few food
items (random) are rotten. There is not much you can do about this or the 
minor effects that it may have, but be warned that rotten things have 
only 1/4 the food value.
'
.H 2 "Tins"
'
Tins take a while to open, and
you will automatically stop opening the tin if a monster approaches
(even if you are under Elbereth).
Tins may contain apple juice (useless) peaches (food), 
rotten meat (causes you to vomit) unknown substance (disgustingly nutritious)
and the important stuff, spinach.  This miracle weed increases your strength  
by a random but potentially large amount.  Two or three of them will 
almost certainly turn you into a hulk. Note that monsters will interrupt
your opening it.  If you don't have a can opener, axes, crysknives, and
daggers help speed up the process of opening tins.  To help you, though, they 
have to be wielded, and if you wield a stickeycursed dagger, tough luck.
'
.H 2 "Food Attribute Index"
'
Levels: satiated, not hungry, hungry, weak, fainting.
.sp
Belly limit: 1499 units
.sp
Initial food in belly: 900 units.
.sp
Probability of rotten food (unless fortune cookie or dead monster): 1/7
.sp
Consequences of rotten food:
.TS
tab(#),center;
c c
l l.
Chance#Result
_
1 in 4#confusion (2d4)
3 in 16#blindness (2d10) (if not blind already)
3 in 16#loss of consciousness (1-10)
3 in 8#no harm
.TE
Value of rotten food: 1/4 normal value.
.sp
Eating restores 1 strength point unless it leaves you weak or satiated.
.P
.so food.tbl
.P
It is possible to use polymorph to change food items.  When you kill Kops
or giants you get cream pies.  They are relatively light and you can
carry many.  When you are hungry simply zap a number (2-3) pies into
food rations.  This allows you to carry a lot of food. (It is a good
idea to always have at least one food ration because you may not be
able to cast.)  You can also polymorph dead bodies with this spell. 
.H 1 "MONSTERS"
'
.H 2 "Monster Data Index"
'
.so monster.tbl
.P
.H 2 "Descriptions"
.LB 4 0 0 2 a
.LI
Acid Blob:  Not edible.  Does not attack unless confused, but when you hit 
it it may splash you with acid or corrode your weapon or armor.  (Elven
cloaks and leather armor are not affected.) Worth 9 XP; good for 
building your levels at first.  However, an acid splash can do as much 
as 8 points of damage.  They are slow and easy to avoid.  If one is 
blocking your path in a corridor or door, you can lead it into a room 
and dodge around it.
.LI
Giant Beetle: Edible.  Nothing else special.
.LI
Cockatrice: Very dangerous!  Any hit will turn you to stone.  Even if it
misses, it has a one in five chance to do so by hissing.  Touching a 
dead cockatrice is also lethal, unless you are wearing gloves.  If you 
threw things at it, do not try to retrieve them until detect food 
tells you there is no corpse underneath.  Don't even think of 
eating it.  Gloves allow you to handle dead cockatrices, but not 
necessarily safely (see gloves).  A monster 
turned to stone by a living cockatrice eventually become a
pile of rocks.  Monsters you hit with a dead one 
seem to be normal corpses.
The wand of cancellation does not stop the petrification.
Typically, the process begins with a message about
your limbs stiffening.  Eating a dead lizard has been known to
prevent petrification.
.LI
Dog: your dog, or that of a previous adventurer.  If you throw tripe or
anything else edible (not rotted corpses) at a wild dog, it becomes tame.
If you leave a tame dog alone on a level, it instantly goes wild.  To
take a tame dog to another level, use stairs when it is adjacent to you.
A tame dog will not attack you unless it is confused.
It can starve to death, and for a while near the end will be confused
by hunger.  If you get ``Sad feelings'' it means 
that your dog has died or been killed.  If you feel ``worried'' it means 
that it's dangerously hungry.  Dogs are surprisingly powerful.  Note: 
if the dog falls down a trap door, it will still be tame \- it only 
goes wild when deserted.  Little dogs grow to be big dogs as the game 
wears on.  There is nothing except a cockatrice that a big dog cannot
at least argue with, and have a reasonable chance of killing.  It seems
to kill only for food, with a few exceptions such as acid blobs and
sometimes kobolds.  If you are fighting something and it has a high 
loyalty quotient, it may get the idea.  
NOTE: Dogs will not step on a cursed object.  Thus you can drop a
cursed object (rings do well in a passageway or door and the dog will not
follow. Later, come back and get him, no worse for the wear. 
(CAVEAT: This may apply only to stickeycursed items.)
.LI
Ettin: in D&D, hard to surprise.  In hack, hard to kill (meaner than a troll).
.LI
Fog Cloud: edible, weak, but difficult to kill. 24XP. A real bargain.
.LI
Gelatinous cube: Occasionally paralyzes. 23XP.
.LI
Homunculus: bite can put you to sleep. (Rare for wizards) Not edible.
.LI
Imp:  Poor attack, but hard to hit.  You might starve to death trying 
to kill one.  Edible.
.LI
Jaguar: has multiple attacks, but does not inflict much damage. 
.LI
Killer Bee:  Like orcs, this denizen of the dungeon appears in swarms.
If you get hit, you may find that the bee's sting is poisoned.  If 
you haven't already eaten a killer bee or a scorpion, doing so
will give you permanent resistance to poison.  They also are created
in swarms by create monster.  Unlike orcs, these are individually
medium-tough monsters, and there is no Orcrist for them.  Poison resistance,
however, takes the sting out of them.
.LI
Leocrotta: The leocrotta is a master of hit and run tactics.  He tends to 
take three hits and then step back.  The leocrotta is a real headache 
when you are fighting other monsters.
.LI
Minotaur: The minotaur is the king of the mazes that exist on the lowest
levels of the dungeon (~27+).  One of these bull-headed creatures
can be found in each maze level. Kill him and you will find a wand of 
digging.
.LI
Nurse: tends to a wear a ring, but not always.
If you remove your armor and go barehanded,
she heals you and raises your maximum hit points.
NOTE: Elbereth works to stop her either way.  You can either be truly
defenseless, heal yourself, or fight her.  Try to avoid her until you
have cleared the level.  It is bad luck to kill a nurse.
.LI
Owlbear: If the owlbear gets a good hit on you, he will embrace you in a
bear hug!  You are then unlikely to be able to escape from its 
clutches short of killing it or teleporting away.
.LI
Piercer: can drop from ceiling by surprise.
If it falls and hits you, it might do up to 30+HP damage.
Once on the ground, however, is not much of a problem.
One of the prime reasons to wear a helmet in hack. Edible. 10XP.
A hanging (hidden) piercer can be found by searching.
.LI
Quivering Blob: Edible.  Weak and slow.
.LI
Giant Rat or Rockmole\(dg: Both are edible.  Rockmoles bore into walls
and corridors, making new passages.
.LI
Giant Scorpion or Spider\(dg: poisonous sting. Edible
for poison resistance.
Like snakes, scorpions and spiders hide
under objects.  You can turn this to your advantage by throwing
missiles at them from across the room.  They will hide under the first
one, then sit still for you to hit with many more.  Beware of spider
webs.
.LI
Tengu: The tengu tends to engage in guerilla tactics (like the leocrotta).
Tengus teleport short distances while in melee, and may even be able
to follow you if you teleport away.
.LI
Unicorn: DON'T fight! this is a good guy.  Throw gems at it.  If the gems
are valuable, the unicorn will give you something valuble.  Throw a gem
directly at it and it "graciously accepts your gift."  This raises
your luck 5 points, which is useful. It won't tell you
this, however.  If you throw worthless glass at it, it will be insulted
and refuse the gift.  Like the albatross, it is unlucky to kill one.
.LI
Violet Fungi: slow, edible.  Sticks to you.
.LI
Long Worm or Wumpus: Has a long body (~) behind a head (w).
Only the head can hit,
though you can hit (and destroy) the worm at any segment. Hitting
the last segment is by far the best approach.
You can get a worm tooth out of the corpse. Enchanting that will
produce a crysknife, a powerful weapon. 
Long worms tend to hit in long bursts; you may get one that
just sits there while you hit it a few times and then hit you 10 or
more times.  The worm scores hits in quantity rather than quality.
Avoid attacking the midsection.
This can divide the long worm in half, creating two short worms.
It is possible to halve a worm, then split each of 
the halves.
Find a good running loop (two doors on the same side of a room
that connect through tunnel) and sneak around to whack at its tail. 
You do not have to kill every single segment to kill the whole worm.  
It is worth 115XP.
.P
Some versions of Hack have the wumpus, a strong creature without
the complications of the long worm.
.LI
Xan: pricks your leg. More than one prick ruins your load-carrying
capacity. Potion of speed or lump or royal jelly will cure.
A real hazard.  Found under
20th level. Good candidate for genocide.  A ring of levitation allows
you to rise above them; they are very short.
.LI
Yellow light: if it hits, it will blind you and disappear.  Blindness is
temporary (much shorter than from a potion of blindness).
(See Floating Eye, and potion of blindness.)
.LI
Zruty: nothing special.  Hits a lot. 
.LE
.LB 3 0 0 2 A
.LI
Giant Ant: poisonous sting can lower your strength.  Moves several times
in one turn.
.LI
Giant Bat: jumps around.  Not powerful, but so fast that it can hit you and
get out of hand-to-hand range. Eating corpse causes confusion like
potion. (No warning message.) Good as dog treat, though.
.LI
Centaur: Fast and powerful.  Edible, but corpse is heavy.
.LI
Dragon: A mega-monster.  Nasty attack or breathes fire. The second best reason
to own a ring of fire resistance at low levels.  You may also do
it the hard way; eating a dragon's corpse 
will give you permanent fire resistance. Unfortunately, a dragon's
corpse is an unliftable object. Dragons can flame you even through walls
and past boulders,
though probably won't hit you. This is particularly common in mazes (level 27+).
A dragon at the back of a zoo can be useful \- they flame at you, and
take out any intervening monsters.  Dragons can flame you through 
Elbereth.  Note that even with fire resistance, the dragon's flames
can cause your scrolls to burn and your potions to explode on you.
.LI
Floating Eye: Gaze can paralyze you, leaving you game for monsters.  It 
cannot do this unless you attack it hand-to-hand.  Eating corpse makes 
you sick but bestows telepathy; whenever you are blind, you
know what and where every monster on the level is.  This turns yellow 
lights and potions of blindness into potential godsends when you have
most of the level mapped out.  To get corpse without getting frozen, 
stand next to it while your dog kills it, then grab corpse before he 
eats it. (Note for Tourists: use your camera to blind it. A blinded 
floating eye cannot defend itself.)  If your dog is deceased, throw
things at it.  Unaffected by wand of cancellation. Wand of invisibility
prevents them from paralyzing just as blindness does.
.LI
Freezing Sphere: eating corpse gives protection from cold as ring, but
permanent (may do poison damage).  Usually gives you a blast.
You may kill a freezing sphere with projectiles/ray wands or missile
weapons. Exploding freezing spheres leave no bodies. It is 
easy to kill at a distance (two crossbow bolts). Ring of cold
resistance will protect you from explosions, as will having eaten a
sphere corpse.
.LI
Gnome: edible.
.LI
Hobgoblin: edible \- good food source.  Attack can do up to 8 points 
of damage. 
.LI
Invisible Stalker: this monster is invisible; the letter is what you
see with ring or potion of see invisible. (Or telepathy)
Eating corpse confuses you and/or allows you to see invisible
thereafter (not to mention turn you invisible for a while!)
.LI
Jackal: usually edible, but seems to rot fast, or perhaps sometimes start
a bit rotten, though not enough to kill you, just confuse you. 
.LI
Keystone Kop: The guys that come to get you when you rob a store.  Do not
eat them!  They are poisonous.  Keep their pies, though.  Killing a
Kop is bad luck.  Pies blind you temporarily, which isn't bad if you
have telepathy.  There is a # wipe command.  Their numbers seem to increase
with the amount stolen.
.LI
Leprechaun: if it hits while you are carying gold, will do no damage but 
will teleport away with a lot of gold.  If it hits while you have
no gold, does damage.  Can also pick up piles of gold, even from under 
your feet. Eating corpse may do poison damage, but bestows teleportation
as with the ring, but permanently. Usually have a pile of gold on them, 
including any they took from you. Good for building up experience in
the lowest levels (40+ XP).
.LI
Mimic: Poses as object in shop or elsewhere.  Step outside shop, the items
that vanish are mimics.  Once they hit you, they can prevent you from
moving off the square until they are dead (or you teleport away).
Edible. (When eaten, you mimic a treasure chest for 30 turns)
In shops, mimics are usually seen as inverted armor, i.e.
armor is ``['' and mimics ``].''
Mimics often mimic doorways (+) or chests of gold ($).  They can be 
discovered by searching, including the ring.  Wand of cancellation will
also show them up.
.LI
Nymph: Seduces you and swipes many possessions, including at least one armor
that she talked you into taking off.  Or, may just grab an item.  
Either way, she teleports away.  Eating corpse is just like eating a
leprechaun corpse.  Kill her, and you get your possessions back.  Ignores
Elbereth. (But see ring of adornment.)  You may ignore her charms if 
you zap her with a wand of cancellation. (NetHack nymphs are very fast and
can pick you clean after a few encounters \- stay away from them.)
.LI
Orc: Always appear in swarms.  If you see one, there are others.
Edible in NetHack.  May have personal treasure.  Scroll 
and wand of monster creation (but not polymorph) may create 8-9 of 
them. Call your weapon Orcrist and whack away.  Can carry items around
that they find on the floor, i.e. in shops.
.LI
Purple Worm: Not that hard to deal with, by the time you get to that
level. They swallow and digest like the trapper, but
you faster, and towards the end you cannot hit.  This is
their main attack, and it is dangerous.
.LI
Quasit or Quantum Mechanic\(dg: Fast (3 spaces and/or attacks per move),
but little damage per blow.  Quantum mechanics will teleport you with
the message ``You are uncertain of your position.''
.LI
Rust Monster:  Rusts armor when it hits.  Items that will rust: all armor
(except leather) and helmets.  Elven
cloaks, shields, and gloves are unaffected.  An elven cloak will
protect the armor underneath it from rusting.  Note that once you 
start taking off your armor, he cannot harm it, unlike rogue. Takes no 
hit points. Try wand of cancellation.  An item may only be rusted to
-3.
.LI
Snake: can hide under items on floor.  Can appear in numbers from fountains.
.LI
Troll: in D&D and rogue, regenerates its wounds. Hack Trolls do not seem
to be as nasty as rogue Trolls. Several hits per move.
.LI
Umber Hulk: chance of confusing you (as potion) if you catch sight of it. 
Save a potion of blindness (and a potion of healing) for the umber
hulk if you detect him beforehand.  Unlike rogue, umber hulks seem
to have more than one chance of confusing you.
.LI
Vampire: Hits from the vampire reduce your maximum hit points,
which is almost as unpleasant as wraiths.
You can drive a vampire away with one hit while wielding a clove of 
garlic.
.LI
Wraith: in D&D, drains one level/hit. Drops you an experience level if it
gets in a good hit. In extreme cases, may take several levels if it
hits well several times. When you kill it, you will go back up one
level if you've lost any (but won't get the extra experience points).
Wraiths usually don't drain levels if you have good armor;
Eat corpse to gain experience level \- one of the fastest and easiest 
ways to increase your level.
.LI
Xorn: In Hack, a xorn is a tough monster, about the same as 
attacking a ettin.
.LI
Yeti: gives cold resistance when eaten.
.LI
Zombie: edible.
.LI @)
Human (you, shopkeeper or guard): Shopkeepers guards will kill you if you
make them mad. They are tough
and ignore Elbereth. If you teleport out with unpaid
items, they will pursue you.
See shops under other for ways of dealing
with them.  Don't try to genocide shopkeepers.  Genociding shopkeepers 
genocides all humans (@). You are a human and thus will vanish too.
Killing humans, including shopkeepers or guards, 
reduces your luck, an internal variable. It also kills telepathy.
(but you can then regain it in the usual manner).
Eating a shopkeeper produces the message: ``You cannibal! You'll be
sorry for this!''
You may encounter a shop on a ghost level that has been robbed by the
ghost. In that case, the shopkeeper is likely to take your money for 
the items stolen by the ghost and even kill you if he's still angry.
.LI ,)
Trapper: The trapper possesses a unique attack
\- it simply swallows and digests you.  After being swallowed by a
trapper, you will get the message ``The trapper digests you'' about
every third turn. You can still hit him from within, and stand a
good chance of killing him. If you don't, after about six (6)
such messages you will get the message "The trapper digests you
totally!".  If such is the case, better luck with your next character.
If you have been swallowed by a trapper, you may polymorph it into 
some other creature whose digestive tract may be less hostile.  
Wands of digging will blast through his
stomach, leaving him very weak. Two bolts of cold didn't seem to
faze him too much. Teleport out if you can. When teleporting out of a 
trapper, a ring of teleport control will let you choose where you want 
to go as usual, but not let you see the level while choosing.
A wand of teleport monster will take you with the trapper. 
.P
Digesting seems to be cumulative, so if you escape one trapper
after 5 digesting messages, be very wary of being swallowed by
another trapper. He may digest you totally in one turn.
.LI &)
Demon: a monster with strength second only to the purple worm,
with a high armor class and good magic 
resistance. (Note: demons are not resistant
to fire.)  Demons replicate; one demon will 
give rise to a second, and a third, etc.  The way to deal with this is 
to back the bunch (or perhaps even just the original) into a dead-end
and start hacking away. Replicas cannot appear unless there is room next
to the replicator. This could also be useful when you have some spare
firepower and need some experience points.
A wand of cancellation will stop or prevent replication.
The wand of undead turning makes them turn tail and run.
.LI ~)
Lurker Above: another swallowing creature.
.LI :)
Chameleon:  Whereas the mimic will mimic objects, the chameleon will
mimic other monsters.  When hard pressed, the chameleon will
revert to its original form. Also unlike the mimic, the chameleon
changes form constantly. So if you see a monster, and then look
again and see a different one, it's probably a chameleon.
Chameleons have all the powers of the monsters 
they emulate. If you see a genocided monster, it is a chameleon.
.LI "`` '')"
(a space) Ghost: incredibly hard to hit.  Does little damage.  Always has
``ghost hoard'' that can be obtained by killing the ghost
or by sneaking around it.  Remember all items in these hoards are
stickeycursed.
Sneaking around a ghost is easier than destroying it, because they
move slowly and do little damage per hit. Find a good running loop.
Don't get caught in a dead end \- you'll either get killed or starve
to death.  If you're getting clobbered by a ghost, do not retreat
toward an up staircase!  If a ghost kills you, you may become a ghost
on the same level.  If many people are ``ghosted''
there, future dungeon-delvers face the grim prospect of several ghosts and
their dogs!  This can get them killed before they can escape, which
aggravates the problem. (It may be advisable to ask root or the games
manager to remove a \fIbones.n\fR level that has reached absurd proportions.)
In addition, If you are about to die on a ghost level, drop
everything; When you become a ghost, your hoard will not be stickeycursed
Ghosts are surprisingly easy to kill if you have a lot of experience
(level 11+). Worth 175XP.
.LI ;)
Giant Eel\(dg: Rumor has it that eels hide under mud and that a unicorn
can clear the water so that you can see them.  They wrap themselves around
you and pull you into the mud to drown.
.LI 1)
Wizard of Yendor: Guardian of the Amulet, a powerful wizard who may
not stay dead (NetHack).
.LI 2)
Mail Daemon: You won't get a crack at this one; this creature delivers
scrolls of mail to you and quickly departs.
.LI 9)
Giant: Powerful creature found on lower levels.  Eating one increases
your strength.
.LE
.H 1 "WEAPONS AND ARMOR"
.H 2 "Weapons"
.P
.so weapon.tbl
'
.H 2 "Armor Items"
'
.TS
box, center, tab (#);
c c c c c
l n n n n.
Name#Chance#Weight#Ac#Protection
_
helmet#3#1#9#0
plate mail#5#5#3#2
splint mail#7#5#4#1
banded mail#9#5#4#0
chain mail#10#5#5#1
scale mail#10#5#6#0
ring mail#12#5#7#0
.TE
.sp
 The armors below do not rust.
.TS
box, center, tab (#);
c c c c c
l n n n n.
Name#Chance#Weight#Ac#Protection
_
studded leather#12#3#7#1
elfin chain mail#1#1#5#3
bronzeplate mail#6#5#4#0
crystalplate mail#1#5#3#2
leather armor#15#3#8#0
elven cloak#5#0#9#3
shield#3#0#9#0
pair of gloves#1#1#9#0
.TE
.P
Protection refers to magical effects.
.P
.H 3 "Elven Cloaks"
Magic cloaks that increase your
armor class by one and protect you from
enchantments.  Elven cloaks do protect your armor from being
dissolved by rust monsters or acid blobs. They can be enchanted.
.H 3 "Helmets"
Prone to digestion by rust monsters,
but they protect you from piercers and from rocks dropping on you
in mazes (if you get that far).
.H 3 "Gloves"
These normally improve your armor class by one, but
their greatest advantage is in preventing you from getting stoned by cockatrice
cadavers.  If you have a wand of wishing, ask for ``a pair of +3 gloves.''
Gloves allow you to pick up and even wield
cockatrice cadavers, turning monsters
to stone right and left until the corpse rots away.
This tactic is not recommended because is very hazardous. If you 
subsequently fall down the stairs (as is usual when heavily loaded) you will 
touch the cadaver and die. Note also that this is inconsiderate of other
players, as any player taking your hoard will encounter
the cockatrice cadaver before they reach the gloves you were using.  If you
have a second pair of gloves, please put them at the bottom of your inventory,
and the cockatrice at the top, except of course for the gloves you are 
wearing. (drop items above it, then pick them up again).
Any grave-robber should reach your spare pair and be overloaded before 
he hits the cadaver, allowing him to put on the gloves.  Note that the cadaver 
will eventually rot away, so don't throw away your weapon.
.H 3 "Armor"
Various types of armor have different basic armor classes, and
any given suit may have a plus or minus in addition.  Minused armor (including
the other items under this heading) is almost always stickeycursed.  Note that
most armor takes some time to put on or take off, and that once you start 
either process, you will follow through no matter what is attacking you.
.H 1 "SCROLLS, POTIONS, RINGS AND WANDS"
'
.H 2 " Scrolls"
'
Unless stated otherwise, all scrolls identify themselves.  
Exception: detect scrolls will give a ``strange feeling'' if there are none of 
their objects on the level, and enchant/damage/destroy weapons/armor will do
the same if you are not wearing any of the item affected.
.P
All scrolls give strange effects when confused or hallucinating.
.P
.TS
tab (#),center,box;
c c c c
l n | l n.
Name#Chance#Name#Chance
_
enchant armor#6#genocide#2#
destroy armor#5#light#10
confuse monster#5#teleportation#5
scare monster#4#gold detection#4
blank paper#3#food detection#1
remove curse#6#identify#18
enchant weapon#6#magic mapping#5
damage weapon#5#amnesia#3
create monster#5#fire#5
taming#1#punishment#1
.TE
.P
.AL
.LI
genocide: everyone's favorite scroll.
Also prevents 
the creation by scroll or wand of more monsters of the type.
Note that genociding cockatrices does not remove their cadavers (so you
still have to watch those ghost treasure hoards).
Also, you can't genocide monsters represented by punctuation
marks (chameleons :, trappers , , demons &, lurkers above ~).
Does not prevent you from polymorphing a monster into the genocided
type or chameleons from assuming that form.
.LI
destroy armor: Destroys outermost layer of armor.  If you are wearing
only armor, it is destroyed.  If you are wearing armor and a helmet,
the helmet is destroyed.  Elven cloak over armor destroys cloak first.
.LI
enchant armor: As destroy armor, but +1 added to enchantment.
Also removes stickeycurse.  Don't overdo it; armor tends to
evaporate if enchanted beyond +3. 
.LI
enchant weapon: weapon in hand or random if barehanded. Otherwise as 
enchant armor, applied to weapons.
.P
Both enchants make item glow green.
There are two kinds of these \- one makes your weapon glow green
for a ``moment,'' the other for a ``while.''  The first is +1, the
second is +2. This also applies to enchant armor, and damage weapon.
If barehanded (or naked skin for armor) your skin itches or hands
twitch. Also applies to damage weapon and destroy
armor.  Enchanting a worm tooth turns it into a crysknife, a very 
powerful weapon.  Overenchanted weapons (> +3) also evaporate.
.LI
damage weapon:  -1 added to weapon. No effect on or of stickeycurse.
Makes item glow black. (see enchant weapon)
Read on a crysknife, this turns it back into a wormtooth, and
reports that your weapon seems duller.
.LI
food/gold/object/monster detection: briefly see all on level.
.LI
magic mapping: all rooms, corridors, secret doors and stairs on level 
revealed. Amnesia is the nemesis.
.LI
amnesia: accumulated map of level lost, except for where you're standing.
In NetHack, the message ``Who was that Maud person anyway'' is displayed.
This is an inside joke, as previous versions say: ``Thinking of Maud
you forget everything else.''
.LI
teleportation: random spot on level.  See ring of teleport control.
Useful in shops.
.LI
fire: reduces your maximum hit points permanently.
.LI
taming: difficult to identify.  If a monster is in sight, it becomes tame,
like your dog, but there is no message. The monster is then equivalent 
to a polymorphed dog.  Thereafter, scroll is named in inventory.
Reading it when surrounded by multiple monsters seems to affect only
the ones nearest you, i.e., a maximum of 8 monsters could be affected.
Warning: If you abandon a tame monster (on a level) it will go wild, 
and will not be pacified by tripe.  Tamed ghosts are possible, but because
they do not eat anything, they cannot be rewarded and hence
eventually return to their accustomed hostile state.
.LI
identify: identify one item.  A few (perhaps 1 in 10) of these will ID 
all items of a type that you own.  Use it on rings first, potions 
second.  Note that if you choose to ID a)ll items, then you had better 
have one of the special types, or you will only get the first item in 
your inventory identified.
.LI
remove curse: gets rid of all stickeycurses on worn items.
``You feel like someone is helping you.''  This scroll is very
important!  Anyone wielding a -3 dart, wearing a cursed ring of 
levitation, or dragging about a heavy iron ball can attest to
this fact.
.LI
create monster: one monster in adjacent square, appropriate to level.
Note if it creates orcs or killer bees, it will make a whole tribe of 
them.
.LI
monster confusion: your hands begin to glow blue.  The next monster you
hit with a hand-to-hand attack will be confused, and your hands will
stop glowing.  A confused monster (assuming it survives the blow that 
confused it) will move in random directions, attacking anything in its 
way.  This includes you (so don't confuse your dog!) or other 
monsters.
.LI
punishment: this cursed scroll outfits you with a ball and chain, 
indicated on the screen by @_0.  The ball, if left to drag, may slow 
you down.  However, it can also be picked up, and if then wielded as a 
weapon, is devastatingly effective.  It is also very heavy, however,
and scrolls of enchant weapon have no effect on it.
The remove curse scroll will cause ball and chain to come loose; you 
can drop them then, but I'd suggest sticking with them if you're 
strong enough.  Also good for getting around fast. Try picking up the 
ball, and throwing it in the direction you want to go.
It is possible that this scroll may reduce your luck
to 0 for the duration.
.LI
scare monster: Similar to rogue in use. If you pick up a scroll that
``turned to dust,'' it \fIwas\fR a scare monster scroll that had already
been dropped.  If you read a scare monster scroll, the message is displayed:
``You hear maniacal laughter in the distance.'' This is the gods laughing
at your stupidity, for you are supposed to drop the scroll on the floor.
Monsters will not attack you when you stand on one, nor will they cross it
if dropped in a corridor. Unlike Elbereth, you can fight from a scare
monster scroll. You can move on and off the scroll if you avoid picking
it up; if you pick it up it will turn to dust.
.LI
blank: this scroll "appears to be blank."  This scroll may be utilized
with a magic marker: write the name of a scroll you are already familiar
with (i.e. one that you've read at least once).
.LE
.P
'
.H 2 "Potions"
'
.S 8 10
.W 7i
.TS
center,box;
c c c
lw(.9i) | lw(2.3i) | lw(3i).
Potion	Standard Message	Special Case(s)
_
restore strength	Wow! This makes you feel great!	T{
... looks sound and hale again! (Thrown to monster)
T}
gain energy	Magical energies course through your body.	T{
You feel feverish. (Confused)
T}
whiskey	Ooph!  This tastes like liquid fire!	T{
You feel somewhat dizzy. (Vapor)
T}
invisibility	Gee!  All of a sudden, your can't see yourself.	T{
For an instant you couldn't see you right hand. (Vapor)
.br
You feel rather airy. (Blinded)
T}
fruit juice	This tastes like fruit juice.
healing	You begin to feel better.	T{
... looks sound and hale again! (Thrown to monster)
T}
paralysis	Your feet are frozen to the floor!	T{
Something seems to be holding you. (Vapor)
.br
You are motionlessly suspended. (Levitated)
T}
monster detection	You sense the presence of monsters.	T{
You feel threatened. (No monsters)
T}
object detection	You sense the presence of objects.	T{
You sense the presence of objects close nearby.
.br
You feel a pull downward. (No objects)
T}
sickness	Yech! This stuff tastes like poison.	T{
... looks rather ill. (Thrown to monster)
T}
confusion	Huh, What?  Where am I?	T{
What a trippy feeling. (Hallucinating)
.br
You feel somewhat dizzy. (Vapor)
T}
gain strength	Wow do you feel strong!	T{
... looks sound and hale again! (Thrown to monster)
T}
speed	You are suddenly moving much faster.	T{
Your knees seem more flexible now. (Vapor)
.br
Your legs get new energy. (Fast)
T}
blindness	A cloud of darkness falls upon you.	T{
It suddenly gets dark. (Vapor)
.br
Bummer! Everything is dark! Help! (Hallucinating)
T}
gain level	You feel more experienced.
extra healing	You feel much better.	T{
... looks sound and hale again! (Thrown to monster)
T}
levitation	You're floating in the air! (Hallucinating)
hallucination	Oh wow! Everything looks so cosmic!	T{
You have a vision for a moment. (Vapor)
T}
holy water	You feel full of awe.	T{
This burns like acid. (When you are a Z,V,W or &)
.br
... shrieks in pain! (Thrown to a Z,V,W,' ' or &)
T}
.TE
.S
.W
'
.H 2 "Rings"
'
.TS
tab (#),center,box;
c c c c
l n | l n.
Name#Chance#Name#Chance
_
adornment#?#protection#?
aggravate monster#?#protection from shape changers#?
cold resistance#?#regeneration#?
conflict#?#searching#?
fire resistance#?#see invisible#?
gain strength#?#stealth#?
hunger#?#teleport control#?
increase damage#?#teleportation#?
levitation#?#warning#?
poison resistance#?##
.TE
.P
.VL 10
.LI
NOTE:
all rings increase your food consumptions to varying degrees.
.LE
.P
.AL
.LI
cold resistance: You are invulnerable to cold, including backblasts from
the wand and freezing spheres.
.LI
fire resistance: You are invulnerable to fire, including backblasts from
the wand and dragon breath.
.LI
teleport control: gives you control of teleport destination, no matter what 
does the teleporting.  Try to teleport into a wall or monster and you get a
random teleport.  Gives no control of when you go, just where.
.LI
regeneration: Regain extra one hit point per round (but increases food
consumption dramatically).
.LI
conflict: causes monsters to attack each other instead of you, if they are 
next to each other.
.LI
hunger: Increases hunger.
.LI
stealth: Monsters don't wake up when you enter a room.
.LI
searching: finds secret doors/traps more easily, and occasionally without 
searching.  Does not necessarily work on first try.
Also finds mimics and piercers.
.LI
increase damage: Increases the damage you do to monsters when hitting them.
.LI
protection: Increases your armor class and probably your (internal) saving
throws.  Can be +1, +2, etc or cursed -1,-2, etc.	
.LI
teleportation: teleports you randomly about every once in a while.  No way 
to control when, but a ring of teleport control determines where.
Without this ring, destination
is random on level. Is usually stickeycursed and uses a great deal of food
energy.
.LI
adornment: useless in rogue, but in Hack it protects against nymphs
(they're dazzled by it and can't attack).
.LI
resistance to poison: You are invulnerable to poison (including poison 
corpses but not rotted ones, and all poison stings, bites, and potions).
does not protect you from rotted (tainted) meat or potions of sickness.
.LI
gain strength: Just what it says.  Increments as with ring of protection.
.LI
protection from shape changers: Forces chameleons to take on their
true form (:).
.LI
warning: glows in various colors, which may be indications of approaching
monsters, traps, etc.  Code unknown.
.LI
levitation: equivalent to a potion of levitation when worn.  Has minor
advantages if you wish to avoid traps, but a cursed ring of levitation
could spell the end of your character!  You are unaffected by traps,
but you cannot pick anything up or go downstairs.
.LI
see invisible: same as potion.
.P
You can wear only one of these items on each hand at any time, so
a stickeycursed ring is a pain when say, you want protection from fire, but 
you're too heavily loaded to take off your increase strength ring, and your 
ring of searching is stuck on.
.LE
'
.H 2 "Wands"
'
.TS
tab (#),center,box;
c c c c
l n | l n.
Name#Chance#Name#Chance
_
light#10#cancellation#5
secret door detection#5#teleportation#5
create monster#5#make invisible#7
wishing#1#probing#2
striking#7#digging#5
nothing#2#magic missile#10
slow monster#5#fire#5
speed monster#5#sleep#5
undead turning#5#cold#5
polymorph#5#death#1
.TE
.P
When a wand is out of charges and you zap it, ``Nothing Happens.''
.P
.AL
.LI
Wishing: you usually get 3 wishes, and wishing for wands of wishing
produces wands with 0 charges left.  When it asks you what you want,
answer with the exact syntax it would appear in your inventory as, if
you had identified it with the scroll of ID.  The wand does not ID
it for you, so be sure and keep track of what you've received and
name the items as necessary.
If you wish for an illegal item or a non-existent item,
you get a random one.  You can wish for any plussed item up to +3,
above that you take your chances.  You can wish for one wand or 3 of
any other non-plussed item with each wish. Sample wishes:
.BL
.LI
+3 plate mail/pair of gloves/helmet/elven cloak/crysknife 
.LI
3 potions of gain level
.LI
ring of regeneration/teleport control(#)
.LI
3 tins (No way to specify spinach, but the odds are good) 
.LE
.P
Wishing for ``a wand of wishing (3)'' will not work.
``Illegal'' wishes that seem to work often are +7 plate, 3 +3 crysknives
(but sometimes gives 3 -3 crysknives)  Your luck may influence the 
chance of getting many items.  In NetHack, it's a good idea to wish for
rare spellbooks.
.LI
Drain Life: usually kills target creature, taking
half of everything else's hit points, including yours.  This is a ray.
.LI
Sleep: of variable duration, and doesn't seem to work on bats.  Some 
monsters get saving throws (i.e., a chance not to be affected). This 
is a ray.
.LI
Fire, Cold, Lightning, Magic Missile: all fire rays/bolts/zaps which do
damage to anything they hit. (See note below.)  Cold seems to do the 
most damage, missiles do the least, fire seems to have the longest range.
.LI
Striking: wand hits the first monster in the direction you point it, even 
from a considerable (but not infinite) distance. Not a ray.
.LI
Polymorph: turns one monster or thing into a random other.
Of course, most people would rather turn a dragon 
into a bat than vice versa.  Use on your dog intil you get a monster 
well above norm for your level, or a mega-monster.  May not change all 
characteristics of the monster (i.e., speed may be unchanged).
Warning: if you polymorph your dog, then leave his level (fall in
a trapdoor, etc) he will not be tamed merely by throwing food at
him (need taming scroll). The dungeon apparently forgets he ever
was your dog.  While it may be fun to polymorph your dog, you are
better advised to use it against any mega-monsters that you may
find at the higher level (e.g. Dragons, demons, trappers).  Your
dog is a fairly strong monster in his own right. 
This wand is almost as useful as digging when you're being digested
by a trapper or a purple worm; have you ever been inside a nymph?
.LI
Make invisible: makes monster invisible, permanently.
Don't use on dog, you'll just keep tripping over him. Easy to
confuse with teleport away. No use whatsoever, except to maybe keep you 
from taking the wand of teleport away for granted.
.LI
Teleport monster: monster teleported to random spot on level.
Check to be sure you don't have a wand of invisibility. Note 
that the monster will usually head straight back home at top speed,
and if he finds you still there, or on the way out, he will be pissed.
.LI
Haste monster: monster moves twice as fast. Use on dog, but watch you
don't trip on him.
.LI
Slow monster: monster moves at half speed. Some monsters get multiple 
attacks every other round (i.e. xorn: 3 claws and bite, or shopkeeper:
2 fists).
.LI
Digging: This produces new corridor sections, and doors where the ray 
intersects a room wall.  Also useful for blasting your way out of a trapper.
.LI
Undead turning: somewhat like fear, but no effect on most monsters.  
Undead are: zombies, ghosts, wraiths, vampires, demons and 
perhaps other monsters. Destroys weaker undead, causes bigger ones 
to flee, including demons.
.LI
Create monster: Creates a monster next to you, just like the scroll.  
This wand may be useful to keep around if you are ever desperately short on
food.  If you are, create a monster, kill it, and feast on the remains!
.LI
Cancellation: Should knock out any magical abilities of the monster. 
Monsters affected: 
.TS
center;
c c
l l.
Monster	Effect
_
Nymphs	cancels her magical allure.
Wraiths	cancels life draining.
Demons 	cancels powers of replication.
Leprechauns	cancels teleport ability
.TE
Monsters which are not affected:
Rust Monsters, Dragons, Invisible Stalkers, Chameleons, Cockatrices,
Floating Eyes.
.LI
Secret door detection: discovers all secret doors in room or in sight in
a corridor.  If there are none, a charge is used and the wand seems
to do nothing.  This wand also detects shape-changers in the
room (e.g. mimics, piercers). Usually comes in 10+ charges.
.LI
Light: lights up dark rooms. Usually comes in 10+ charges.
.LI
Probing: Gives stats on a creature in the direction you indicate. Tells
hit points, maximum, how much gold.
.LE
.H 3 "Don't shoot your own foot off."
Note that ray/straight-line wands are best fired when you are 
diagonally aligned with your target so that ricochets can't hit you.
ricochets can be used to hit targets that won't get in straight line of fire 
(leprechauns, shopkeepers, etc.), or to get two hits on a
target.  Shots into corners 
bounce straight back at you.  Doors can reflect a diagonal shot back at you, 
and the wand of fire reflects diagonally back at you occasionally just for the 
heck of it.  Wands seem to have differing, and possibly partly random, maximum
ranges.
.H 3 "Test them first."
When you get a wand, try zapping it.  If it is non-directional, (like
wishing) you will usually find out what it is immediately.  If not, hit
<ESC> when it asks you what direction, and save it until you reach a monster.
This lets you ID many wands before you find a monster to test 
them out on, and improves the odds that a partly-tested, directional wand
will turn out to be offensive, rather than secret door detection or light.
.H 1 "EXTENDED COMMAND SET"
These commands are accessed by first entering a pound sign (#)
followed by the command or a question mark.  Not found in all
versions of Hack.
.VL 10
.LI #dip
Use for every potion that you want to quaff the #dip command, 
dip arrows, bolts or darts in the potion and if it was a potion of :
Sickness or Paralysis: your dipped weapon gets stronger.
Holy water: your weapon gets blessed and is better in use 
against the undead.  Note that positive potions have a 
positive effect on monsters.  The dip command can also be used
with fountains.  (see ``Fountains'' and ``Excalibur.'') 
.LI #pray
You can pray after intervals of approximately 500 turns.
Praying restores strength, fills the stomach and heals.
You can use the option time to see how much time has passed.
Nothing bad will happen, but be sure that your time has come.
.LI #sit
Use this command while you are on an throne (\).
You might get a wish (Chance 1 in 86) but also you can lose 
your gold or your possessions get cursed or identified.
.LI #wipe
Clean your face when dirty.
.LI #breathe 
First polymorph yourself in a Dragon.
.LI #remove
First polymorph into a Nymph
.LI #cast
Cast spell. See next section.
'
.H 2 "Spells and Spellbooks"
'
Spells are found in spellbooks and learned via the ``X'' command.
Save extra spellbooks; if you read a scroll of amnesia you may
forget all your spells.  Most spellbooks are booby-trapped with
poison or other effects (``You feel threatened'' accompanied by paralysis)
and yield no spell.
.P
.TS H
tab (#),center,box;
c c c c c c
l n n | l n n.
Name#Chance#Level#Name#Chance#Level
_
.TH
magic missile#4#2#detect unseen#4#3
fireball#2#4#extra healing#3#3
sleep#6#1#charm monster#3#3
cone of cold#1#5#levitation#3#4
finger of death#1#7#restore strength#2#4
healing#6#1#invisibility#3#4
detect monsters#5#1#detect treasure#3#4
force bolt#4#1#dig#2#5
light#5#1#remove curse#2#5
confuse monster#5#2#magic mapping#2#5
cure blindness#3#2#identify#1#5
slow monster#4#2#turn undead#1#6
create monster#4#2#polymorph#1#6
detect food#5#2#create familiar#1#6
haste self#3#3#teleport away#2#6
cause fear#4#3#cancellation#1#7
cure sickness#3#3#genocide#1#7
.TE
.P
There most powerful spellbooks are, in order of importance: Polymorph,
Genocide, Identify, Charm Monster, Magic Mapping, Digging and Finger of
Death.
.P
The polymorph spell works on objects as well as monsters, although
the object type remains consistent.  You can lay down a pile of objects
and cast the spell until you collect those desired.  There is no
limit to the number of objects you can polymorph at one time.
A high level characters can even polymorph themselves.  To accomplish
this, enter a period (.) when prompted for the direction.  There are
certain disadvantages to self-polymorphing.  If you polymorph into a
non-humanoid creature, you will not be able to use your weapon or
armor.  Although you inherit all the creatures characteristics, this
may also include low hit points. 
.P
Note that healing and extra healing do not increase your hitpoints
past the maximum (in NetHack 1.4 or later).
.H 1 "ITEMS (Tools, Gems, etc.)"
.H 2 "Tools"
Tools are ``applied'' with the ``a'' command.
'
.H 3 "Whistles"
When you blow an ordinary whistle (syntax is ``a whistle'')
you produce a high whistling sound and your dog,
immediately heads in your direction.  Particularly useful if you are
teleporting randomly.  Magic whistles (syntax is ``magic 
whistle'') produce a strange whistling sound, and teleport your dog to you. 
Be careful, whistling may also wake up other denizens of the level.
.P
.H 3 "Icebox"
This is the Archaeologist/Speleologist's ``large box.''  Corpses
promptly placed in it will not spoil.  It is, however, incredibly heavy and
you must be carrying both the box and the corpse to use it.  You may want to 
leave it on the first level and go back for it when you've had some spinach.
.P
.H 3 "Camera"
Tourists start out with this one. Flash in the direction of a
monster to temporarily dazzle the monster.
.P
.H 3 "Pickaxe"
Archaeologist's handy tool.  Can be used to dig down or
around.  Will even get in a shop if you're persistent, though the shopkeeper
will ask you to leave it outside.
.P
.H 3 "Leash"
Supposedly quite handy, though code isn't clean yet (dropping the leash
still leaves fido tethered to you).
.H 3 "Can Opener"
The best thing to open tins with.
'
.H 2 "Amulet" 
'
The Amulet of Yendor, the lengendary item from the days of Rogue, is
found on level 40, guarded by the Wizard of Yendor and his faithful
Hell Hound. (See the section on lower level ``Strategy'' for a
discussion of obtaining the Amulet.)
You can also wish the Amulet, but you will get a cheap
imitation amulet instead.
'
.H 2 "Gems"
'
.TS
tab(#),center, box;
c c c
l n n.
Name#Chance#Value
_
dilithium crystal#1#4500
diamond#1#4000
ruby#1#3500
sapphire#1#3000
emerald#1#2500
turquoise#1#2000
aquamarine#1#1500
tourmaline#1#1000
topaz#1#900
opal#1#800
garnet#1#700
amethyst#1#650
agate#2#600
onyx#2#550
jasper#2#500
jade#2#450
.TE
.H 1 "TRICKS AND OTHER NON-INTUITIVE ACTIONS"
.H 2 "Elbereth"
If you E)ngrave Elbereth with your bare hands (-), it acts
as a sign of peace.  Most monsters are then unable to attack you as
long as you do not move off the square or fight a monster next to you.
Exceptions are dragons, which breathe fire, angry shopkeepers, and
(in NetHack only) any creature normally armed with a projectile weapon:
.TS
center;
c c
l l.
Monster	weapon
_
centaurs	crossbow bolts
orcs	darts
kops	cream pies
.TE
(In Nethack, you cannot throw
things from an Elbereth, but you can still use wands. There is also a
chance of inadvertently smudging the word, thus breaking the spell.)
Other than movement or hand-to-hand fighting, you 
can do anything on an Elbereth, such as quaffing potions, reading scrolls, etc.
.P
Note that if you step off the square, or violate it by fighting, the letters 
are disturbed and the inscription is no good anymore.  However, you can write
it again on the next turn, even on the same square.  If you teleport off the
square, you are no longer protected, but if you find it again, it will be 
undisturbed and potent.  A NetHack Magic Marker can draw an indelible
Elbereth that you can fight from.
.H 2 "Orcrist"
If your weapon in hand is named Orcrist, it gets a hefty bonus
on damage (rumored to be +10) against orcs.
.H 2 "Excalibur"
Dipping a long sword you have called Excalibur is rumored to eventually
yield a +5 long sword (after some initial damage).  There are possible
complications (see ``Fountains'').
.H 2 "Hit Points and Nurses"
Ironically, nurses do a great deal of damage in battle.  However, if you
unwield your weapon and strip yourself of all armor, a nurse will heal
you.  She will even go so far as to continue past your maximum hit
points.  It is possible to follow a nurse around and gain scores of
hit points.
.H 2 "Your Score"
Beyond mere survival, an important aspect of the game is your score.
You get points for gold, gems (if you leave the dungeon with 
them), and the Amulet.  (The first amulet is worth 5000, each successive 
(genuine) amulet doubles your score.) You also seem to get points for the 
following things:  levels descended to (so dungeon-diving may help), 
and magic items identified or possibly just tried out (or possibly 
just accumulated). And also, experience points, seemingly 4-for-1.
Leaving the dungeon with a pet (especially an experienced one) is
also worth points.  Mapping out rooms, and possibly discovering
secret doors may add to your score.
.H 2 "Luck"
This is an internal variable that can make life in the dungeon
much easier or much harder.  Little is known about it, but it probable affects
your chances of finding secret doors, waking up monsters, saving vs. magic,
and possibly hitting and being hit by monsters.  Giving gems to a unicorn is
increases your luck by one point, while killing/eating humans, unicorns, dogs
kops and nurses decreases your luck by \fIfive\fR points.  Throwing a worthless
piece of glass to a unicorn costs you one point, as does playing during
the unlucky phase of the moon.  Note that when the moon is full, you see
the message ``You are lucky, full moon tonight!'' and your luck is
\fIincreased\fR by one point.
It is also rumored that there is also a small chance to 
decrease it slightly by killing defenseless (blinded) monsters.
The absolute range of luck is -10 to +10, and you begin with a value
from 1 to 4, inclusive (ignoring the phase of the moon).
.bp
.ce 1
\fBAPPENDIX A: HACK REFERENCE SHEET\fR
.S 8 10
.TS
tab (#),center,expand,box;
c s s s s s
c c c c c c
c l | c l | c l.
COMMAND SUMMARY
_
\fIx\fR#Command#\fIx\fR#Command#\fIx\fR#Command
_
a#apply, use#j#move down#s#search
b#move down-left#k#move up#t#throw in direction
c#call item#l#move right#u#move up-right
d#drop item#m#skip over#v#program version
e#eat food#n#move down-right#w#wield weapon
<#up stairs#o#display options#x#list spells
>#down stairs#p#pay off shopkeeper#:#look
h#move left#q#quaff potion#z#zap wand
i#inventory#r#read scroll#?#this list
_
A#take off some armor#J#run down#S#save game
B#run down-left#K#run up#T#take off armor
C#name something#L#run right#U#run up-right
D#drop objects#,#pick up all#V#game history
E#engrave#N#run down-right#W#wear armor
^R#redraw screen#O#set options#X#transcribe
^P#last message#P#put on ring#Y#run up-left
H#run left#Q#quit game#\e#list discovered items
I#select inventory#R#take off ring#/#identify
.TE
.P
.TS
tab (#),center,expand,box;
c c c c c c c c c
c l l | c l l | c l l.
N#Name#Attack#N#Name#Attack#N#Name#Attack
_
A#giant ant#saps strength#V#vampire#lowers max. HP#r#rockmole
B#bat##W#wraith#takes levels#s#scorpion#poison stings
C#centaur##X#xorn#+hits#s#giant spider
D#dragon#flames#Y#yeti##t#tengu#teleports
E#floating eye#transfixes#Z#zombie##u#unicorn#friendly
F#freezing sphere#cold blasts#a#acid blob#corrodes#v#violet fungi#holds
G#gnome##b#giant beetle##w#long worm#divides
H#hobgoblin##c#cockatrice#petrifies#w#wumpus
I#invisible stalker##d#dog##x#xan#pricks
J#jackal##e#ettin#tough#y#yellow light#blinds
K#keystone kop#throws pies#f#fog cloud##z#zruty
K#kobold##g#gel. cube#freezes# #ghost#tough
L#leprechaun#steals gold#h#homunculus#sleep#@#shopkeeper#tough
M#mimic#chest(])#i#imp##`#lurker above#swallows
N#nymph#steals items#j#jaguar##,#trapper#swallows
O#orc#uses darts#k#killer bee#poison stings#:#chameleon#changes
P#purple worm#swallows#l#leocrotta##;#giant eel
Q#quasit#FAST#m#minotaur##&#demon#replicates
Q#quantum mech.#teleports you#n#nurse#heals#1#Wizard#magic
R#rust monster#rusts armor#o#owlbear#hugs#2#mail daemon#delivers mail
S#snake#poison bites#p#piercer#drops#9#giant#throws boulders
T#troll#regenerates#q#quiv. blob#
U#umber hulk#confuses#r#giant rat#
.TE
.P
.TS
tab (%),center,expand,box;
c s s s s s
c c c c c c
l l || l n | l n.
EXTENDED COMMAND SET/EXPERIENCE LEVELS
_
Command%Effect%Level%Points%Level%Points
_
#breathe:%to breathe fire%1 =%initial%8 =%640
#cast:%to cast spells%2 =%10%9 =%1280
#dip:%to dip an item%3 =%20%10 =%2560
#pray:%to pray%4 =%40%11 =%5120
#remove:%to steal an item%5 =%80%12 =%10240
#sit:%to sit on a throne%6 =%160%13 =%20480
#wipe:%to wipe eyes%7 =%320%14 =%40960
.TE
.S
.TC
