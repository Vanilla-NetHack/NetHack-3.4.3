                       A Guide to the Mazes of Menace

                               Eric S. Raymond
        (Extensively edited and expanded for 3.0 by Mike Threepoint)
                             Thyrsus Enterprises
                              Malvern, PA 19355

*** 1.  Introduction

    You  have just finished your years as a student at the local adventurer's
guild.  After much  practice  and  sweat  you  have  finally  completed  your
training  and  are  ready to embark upon a perilous adventure.  To prove your
worthiness, the local guildmasters have sent you into the  Mazes  of  Menace.
Your  quest is to return with the Amulet of Yendor.  According to legend, the
gods will grant immortality to the one who recovers this  artifact;  true  or
not,  its recovery will bring honor and full guild membership (not to mention
the attentions of certain wealthy wizards).

    Your abilities and strengths for dealing with the  hazards  of  adventure
will vary with your background and training:

Archeologists
        understand dungeons pretty well; this enables them  to  move  quickly
        and  sneak  up  on  dungeon nasties.  They start equipped with proper
        tools for a scientific expedition.

Barbarians
        are  warriors  out of the hinterland, hardened to battle.  They begin
        their quests with naught but uncommon strength, a trusty hauberk, and
        a great two-handed sword.

Cavemen and Cavewomen
        start with exceptional strength and neolithic weapons.

Elves
        are  agile,  quick,  and  sensitive; very little of what goes on will
        escape an Elf.  The quality of Elven craftsmanship often  gives  them
        an advantage in arms and armor.

Healers
        are wise in medicine and the apothecary.  They  know  the  herbs  and
        simples  that  can  restore  vitality,  ease  pain,  anesthetize, and
        neutralize poisons; and with their instruments,  they  can  divine  a
        being's  state  of  health or sickness.  Their medical practice earns
        them quite reasonable amounts of money, which they enter the  dungeon
        with.

Knights
        are distinguished from the common skirmisher by their devotion to the
        ideals of chivalry and by the surpassing excellence of their armor.

Priests and Priestesses
        are clerics militant, crusaders advancing the cause of  righteousness
        with  arms,  armor,  and arts thaumaturgic.  Their ability to commune
        with deities via prayer occasionally extricates them from peril,  but
        can also put them in it.

Rogues
        are agile and stealthy thieves, who carry daggers,  lock  picks,  and
        poisons to put on darts.

Samurai
        are the elite warriors of feudal Nippon.  They  are  lightly  armored
        and  quick,  and  wear  the  dai-sho,  two  swords  of  the deadliest
        keenness.

Tourists
        start  out  with  lots of gold (suitable for shopping with), a credit
        card, lots of food,  some  maps,  and  an  expensive  camera.    Most
        monsters don't like being photographed.

Valkyries
        are hardy warrior women.  Their upbringing in  the  harsh  Northlands
        makes  them  strong and inures them to extremes of cold, and instills
        in them stealth and cunning.

Wizards
        start  out  with a fair selection of magical goodies and a particular
        affinity for dweomercraft.

    You set out for the dungeon and after several days of uneventful  travel,
you  see the ancient ruins that mark the entrance to the Mazes of Menace.  It
is late at night, so you make camp  at  the  entrance  and  spend  the  night
sleeping  under  the  open  skies.  In the morning, you gather your gear, eat
what may be your last meal outside, and enter the dungeon.

*** 2.  What is going on here?

    You have just begun a game of NetHack.  Your goal  is  to  grab  as  much
treasure  as  you can, retrieve the Amulet of Yendor, and escape the Mazes of
Menace alive.  On the screen is kept a map of where you have  been  and  what
you have seen on the current dungeon level; as you explore more of the level,
it appears on the screen in front of you.

    When NetHack's ancestor rogue first appeared, its screen orientation  was
almost  unique  among computer fantasy games.  Since then, screen orientation
has become the norm rather than the exception; NetHack  continues  this  fine
tradition.  Unlike text adventure games that input commands in pseudo-English
sentences and explain the results in words, NetHack commands are all  one  or
two  keystrokes  and  the results are displayed graphically on the screen.  A
minimum screen size of 24 lines by 80 columns is recommended; if  the  screen
is larger, only a 21x80 section will be used for the map.

    NetHack  generates a new dungeon every time you play it; even the authors
still find it an entertaining and exciting game despite  having  won  several
times.

*** 3.  What do all those things on the screen mean?

    In  order  to  understand  what  is  going  on in NetHack, first you must
understand what NetHack is  doing  with  the  screen.    The  NetHack  screen
replaces  the  ``You see...'' descriptions of text adventure games.  Figure 1
is a sample of what a NetHack screen might look like.
-----------------------------------------------------------------------------
The bat bites!

              ------
              |....|    ----------
              |.<..|####...@...$.|
              |....-#   |...B....+
              |....|    |.d......|
              ------    -------|--



Player the Rambler         St:12  Dx:7  Co:18  In:11  Wi:9  Ch:15  Neutral
Dlvl:1  G:0  HP:9(12)  Pw:3(3)  AC:10  Xp:1/19  T:257  Weak
-----------------------------------------------------------------------------
                                  Figure 1
-----------------------------------------------------------------------------

*** 3.1.  The status lines (bottom)

    The bottom two lines of the screen  contain  several  cryptic  pieces  of
information  describing  your  current status.  If either status line becomes
longer than the width of the screen, you might not see all of it.   Here  are
explanations of what the various status items mean (though your configuration
may not have all the status items listed below):

Rank
        Your   character's  name  and  professional  ranking  (based  on  the
        experience level, see below).

Strength
        A  measure  of  your  character's  strength,  one  of  your six basic
        attributes.  Your  attributes  can  range  from  3  to  18  inclusive
        (occasionally  you  may  get super-strengths of the form 18/xx).  The
        higher your strength, the stronger you are.    Strength  affects  how
        successfully you perform physical tasks and how much damage you do in
        combat.

Dexterity
        affects  your  chances to hit in combat, to avoid traps, and do other
        tasks requiring agility or manipulation of objects.

Constitution
        affects  your  ability  to withstand injury and other strains on your
        stamina.

Intelligence
        affects your ability to cast spells.

Wisdom
        comes from your religious affairs.  It affects your magical energy.

Charisma
        affects  how  certain  creatures react toward you.  In particular, it
        can affect the prices shopkeepers offer you.

Alignment
        Lawful,  Neutral,  or Chaotic.  Basically, Lawful is good and Chaotic
        is evil.  Your alignment influences how other monsters  react  toward
        you.

Dungeon Level
        How deep you have gone into the  dungeon.    It  starts  at  one  and
        increases as you go deeper into the dungeon.  The Amulet of Yendor is
        reputed to be somewhere beneath the twentieth level.

Gold
        The number of gold pieces you have.

Hit Points
        Your current and maximum hit points.  Hit points  indicate  how  much
        damage you can take before you die.  The more you get hit in a fight,
        the lower they get.  You can regain  hit  points  by  resting.    The
        number  in  parentheses  is  the  maximum  number your hit points can
        reach.

Power
        Spell  points.  This tells you how much mystic energy (mana) you have
        available for spell casting.  When you type `+' to list your  spells,
        each will have a spell point cost beside it in parentheses.  You will
        not see this if your dungeon has been set up without spells.

Armor Class
        A  measure  of how effectively your armor stops blows from unfriendly
        creatures.  The lower this number is, the more effective  the  armor;
        it is quite possible to have negative armor class.

Experience
        Your  current  experience  level  and  experience  points.    As  you
        adventure,  you  gain experience points.  At certain experience point
        totals, you gain an experience level.  The more experienced you  are,
        the  better  you  fight and withstand magical attacks.  Many dungeons
        show only your experience level here.

Time
        The  number  of  turns elapsed so far, displayed if you have the time
        option set.

Hunger Status
        Your  current  hunger status, ranging from Satiated down to Fainting.
        If your hunger status is normal, it is not displayed.

    Additional status flags may appear after the hunger status:    Conf  when
you're confused, Sick when sick, Blind when you can't see, Stun when stunned,
and Hallu when hallucinating.

*** 3.2.  The message line (top)

    The top line of the screen is reserved for messages that describe  things
that  are impossible to represent visually.  If you see a ``--More--'' on the
top line, this means that NetHack has  another  message  to  display  on  the
screen,  but  it wants to make certain that you've read the one that is there
first.  To read the next message, just press the space bar.

*** 3.3.  The map (rest of the screen)

    The rest of the screen is the map of the level as you have explored it so
far.    Each  symbol  on  the  screen  represents something.  You can set the
graphics option to change some of the symbols the game uses;  otherwise,  the
game  will  use  default symbols.  Here is a list of what the default symbols
mean:

- and | The walls of a room, or an open door.

.       The floor of a room, or a doorless doorway.

#       A corridor, or possibly a kitchen sink or drawbridge (if your dungeon
        has sinks or drawbridges).

<       A way to the previous level.

>       A way to the next level.

+       A  closed  door, or a spell book containing a spell you can learn (if
        your dungeon has spell books).

@       A human (you, usually).

$       A pile of gold.

^       A trap (once you detect it).

)       A weapon.

[       A suit or piece of armor.

%       A piece of food (not necessarily healthy).

?       A scroll.

/       A wand.

=       A ring.

!       A potion.

(       A useful item (pick-axe, key, lamp...).

"       An amulet, or a spider web.

*       A gem or rock (possibly valuable, possibly worthless).

`       A boulder or statue.

0       An iron ball.

_       An altar (if your dungeon has altars), or an iron chain.

}       A pool of water or moat.

{       A fountain (your dungeon may not have fountains).

\       An opulent throne (your dungeon may not have thrones either).

a-zA-Z  and other symbols.  Letters and certain other symbols  represent  the
        various  inhabitants  of the Mazes of Menace.  Watch out, they can be
        nasty and vicious.  Sometimes, however, they can be helpful.

    You need not memorize all these symbols; you can ask the  game  what  any
symbol  represents  with  the  `/' command (see the Commands section for more
info).

*** 4.  Commands

    Commands are given to NetHack by typing one or  two  characters;  NetHack
then asks questions to find out what it needs to know to do your bidding.

    For  example,  a  common question, in the form ``What do you want to use?
[a-zA-Z ?*]'', asks you  to  choose  an  object  you  are  carrying.    Here,
``a-zA-Z''  are  the  inventory letters of your possible choices.  Typing `?'
gives you an inventory list of these items, so you can see what  each  letter
refers  to.    In  this  example, there is also a `*' indicating that you may
choose an object not on the list, if you wanted to use something  unexpected.
Typing  a  `*'  lists  your  entire  inventory,  so you can see the inventory
letters of every object you're carrying.  Finally, if you  change  your  mind
and decide you don't want to do this command after all, you can press the ESC
key to abort the command.

    You can put a number before most commands to repeat them that many times;
for  example,  ``10s''  will  search  ten  times.  If you have the number_pad
option set, you must type `n' to prefix a count, so the example  above  would
be  typed  ``n10s''  instead.  Commands for which counts make no sense ignore
them.  In addition, movement commands can be  prefixed  for  greater  control
(see below).  To cancel a count or a prefix, press the ESC key.

    The  list  of  commands  is  rather  long, but it can be read at any time
during the game through the `?' command, which accesses  a  menu  of  helpful
texts.  Here are the commands for your reference:

?       Help menu:  display one of several help texts available.

/       Tell  what a symbol represents.  You may choose to specify a location
        or type a symbol (or even a whole word)  to  define.    If  the  help
        option  is  on,  and  NetHack  has  some special information about an
        object or monster that you looked at, you'll be  asked  if  you  want
        ``More  info?''.    If  help is off, then you'll only get the special
        information if you explicitly ask for it by typing in the name of the
        monster or object.

&       Tell what a command does.

<       Go up a staircase to the previous level (if you are on the stairs).

>       Go down a staircase to the next level (if you are on the stairs).

[yuhjklbn]
        Go one step in the direction indicated (see Figure 2).  If there is a
        monster  there,  you  will  fight  the  monster  instead.  Only these
        one-step movement commands cause you to fight  monsters;  the  others
        (below) are ``safe.''

-----------------------------------------------------------------------------
             y  k  u                7  8  9
              \ | /                  \ | /
             h- . -l                4- . -6
              / | \                  / | \
             b  j  n                1  2  3
                             (if number_pad is set)
-----------------------------------------------------------------------------
                                  Figure 2
-----------------------------------------------------------------------------

[YUHJKLBN]
        Go in that direction until you hit a wall or run into something.

m[yuhjklbn]
        Prefix:  move without picking up any objects.

M[yuhjklbn]
        Prefix:  move far, no pickup.

g[yuhjklbn]
        Prefix:  move until something interesting is found.

G[yuhjklbn]
        Prefix:  same as `g', but forking  of  corridors  is  not  considered
        interesting.

.       Rest, do nothing for one turn.

a       Apply (use) a tool (pick-axe, key, lamp...).

A       Remove  all  armor.  Use `T' (take off) to take off only one piece of
        armor.

^A      Redo the previous command.

c       Close a door.

C       Call (name) an individual monster.

^C      Panic button.  Quit the game.

d       Drop something.  Ex. ``d7a'' means drop seven items of object a.

D       Drop several things.  In answer  to  the  question  ``What  kinds  of
        things  do  you want to drop? [!%= au]'' you should type zero or more
        object symbols possibly followed by `a' and/or `u'.

        Da - drop all objects, without asking for confirmation.
        Du - drop only unpaid objects (when in a shop).
        D%u - drop only unpaid food.

^D      Kick something (usually a door).

e       Eat food.

E       Engrave a message on the floor.  Engraving the word ``Elbereth'' will
        cause  most  monsters  to  not  attack  you  hand-to-hand (but if you
        attack, you will rub it out); this is often useful to give yourself a
        breather.    (This  feature  may be compiled out of the game, so your
        version might not necessarily have it.)

        E- - write in the dust with your fingers.

i       List your inventory (everything you're carrying).

I       List selected parts of your inventory.

        I* - list all gems in inventory;
        Iu - list all unpaid items;
        Ix - list all used up items that are on your shopping bill;
        I$ - count your money.

o       Open a door.

O       Set  options.    You  will  be asked to enter an option line.  If you
        enter a blank line, the current options are reported.   Entering  `?'
        will  get  you  explanations  of the various options.  Otherwise, you
        should enter a list of options separated by commas.    The  available
        options  are listed later in this Guidebook.  Options are usually set
        before the game, not with the `O' command; see the section on options
        below.

p       Pay your shopping bill.

P       Put on a ring.

^P      Repeat previous message (subsequent ^P's repeat earlier messages).

q       uaff (drink) a potion.

Q       Quit the game.

r       Read a scroll or spell book.

R       Remove a ring.

^R      Redraw the screen.

s       Search  for  secret  doors  and  traps  around you.  It usually takes
        several tries to find something.

S       Save the game.  The game will be restored automatically the next time
        you play.

t       Throw an object or shoot a projectile.

T       Take off armor.

^T      Teleport, if you have the ability.

v       Display version number.

V       Display the game history.

w       Wield weapon.  w- means wield nothing, use your bare hands.

W       Wear armor.

x       List the spells you know (same as `+').

X       Enter explore (discovery) mode.

z       Zap a wand.

Z       Zap (cast) a spell.

^Z      Suspend the game (UNIX(R) versions with job control only).
        (R)UNIX is a registered trademark of AT&T.

:       Look at what is here.

,       Pick up some things.

@       Toggle the pickup option on and off.

^       Ask for the type of a trap you found earlier.

)       Tell what weapon you are wielding.

[       Tell what armor you are wearing.

=       Tell what rings you are wearing.

"       Tell what amulet you are wearing.

(       Tell what tools you are using.

$       Count your gold pieces.

+       List the spells you know (same as `x').

\       Show what types of objects have been discovered.

!       Escape to a shell.

#       Perform an extended command.  As you can see, the authors of  NetHack
        used  up  all  the  letters,  so  this is a way to introduce the less
        useful commands, or commands used under limited circumstances.    You
        may  obtain  a  list of them by entering `?'.  What extended commands
        are available depend on what features the game was compiled with.

        If your keyboard has a meta key (which, when pressed  in  combination
        with  another key, modifies it by setting the `meta' [8th, or `high']
        bit), you can invoke the extended  commands  by  meta-ing  the  first
        letter  of  the  command.  In PC and ST NetHack, the `Alt' key can be
        used in this fashion.

M-a     Adjust inventory letters (the fixinvlet option must be ``on''  to  do
        this).

M-c     Talk to someone.

M-d     Dip an object into something.

M-f     Force a lock.

M-j     Jump to another location.

M-l     Loot a box on the floor.

M-m     Use a monster's special ability.

M-N     Name an item or type of object.

M-o     Offer a sacrifice to the gods.

M-p     Pray to the gods for help.

M-r     Rub a lamp.

M-s     Sit down.

M-t     Turn undead.

M-u     Untrap something (usually a trapped object).

M-v     Print compile time options for this version of NetHack.

M-w     Wipe off your face.

    If the number_pad option is on, additional letter commands are available:

j       Jump to another location.  Same as ``#jump'' or ``M-j''.

k       Kick something (usually a door).  Same as `^D'.

l       Loot a box on the floor.  Same as ``#loot'' or ``M-l''.

N       Name an item or type of object.  Same as ``#name'' or ``M-N''.

u       Untrap a trapped object or door.  Same as ``#untrap'' or ``M-u''.

*** 5.  Rooms and corridors

    Rooms  in  the  dungeon  are  either lit or dark.  If you walk into a lit
room, the entire room will be drawn on the screen.  If you walk into  a  dark
room,  only  the  areas  you can see will be displayed.  In darkness, you can
only see one space in all directions.  Corridors are always dark, but  remain
on the map as you explore them.

    Secret  corridors  are  hidden.   You can find them with the `s' (search)
command.

*** 5.1.  Doorways

    Doorways connect rooms and corridors.  Some doorways have no  doors;  you
can  walk  right  through.    Others  have  doors in them, which may be open,
closed, or locked.  To open a closed door, use the  `o'  (open)  command;  to
close it again, use the `c' (close) command.

    You  can  get through a locked door by using a tool to pick the lock with
the `a' (apply) command, or by kicking it open with the `^D' (kick) command.

    Open doors cannot be entered diagonally; you must approach them  straight
on, horizontally or vertically.  Doorways without doors are not restricted.

    Doors can be useful for shutting out monsters.  Most monsters cannot open
doors, although a few don't need to (ex. ghosts can walk through doors).

    Secret doors are hidden.   You  can  find  them  with  the  `s'  (search)
command.

*** 5.2.  Traps (`^')

    There  are  traps throughout the dungeon to snare the unwary delver.  For
example, you may suddenly fall into a pit and  be  stuck  for  a  few  turns.
Traps  don't  appear  on your map until you trigger one by moving onto it, or
you discover it with the `s' (search) command.  Monsters  can  fall  prey  to
traps, too.

*** 6.  Monsters
    Monsters  you  cannot  see are not displayed on the screen.  Beware!  You
may suddenly come upon one in a dark place.  Some magic items  can  help  you
locate them before they locate you, which some monsters do very well.

*** 6.1.  Fighting

    If  you see a monster and you wish to fight it, just attempt to walk into
it.  Many monsters you find will mind their own business  unless  you  attack
them.    Some of them are very dangerous when angered.  Remember:  Discretion
is the better part of valor.

*** 6.2.  Your pet

    You start the game with a little dog (`d') or cat  (`f'),  which  follows
you about the dungeon and fights monsters with you.  Like you, your pet needs
food to survive.  It usually feeds itself on fresh carrion and  other  meats.
If  you're  worried  about  it  or want to train it, you can feed it, too, by
throwing it food.

    Your pet also gains experience from killing monsters, and can  grow  over
time, gaining hit points and doing more damage.  Initially, your pet may even
be better at killing things than you, which makes pets useful  for  low-level
characters.

    Your  pet  will  follow  you up and down staircases, if it is next to you
when you move.  Otherwise, your pet will be stranded, and may become wild.

*** 6.3.  Ghost levels

    You may encounter the shades and corpses of other  adventurers  (or  even
former  incarnations  of  yourself!)  and their personal effects.  Ghosts are
hard to kill, but easy to avoid, since they're slow  and  do  little  damage.
You  can  plunder  the  deceased  adventurer's possessions; however, they are
likely to be cursed.  Beware of whatever killed the former player.

*** 7.  Objects

    When you find something in the dungeon, it is common to want to  pick  it
up.    In  NetHack,  this  is  accomplished automatically by walking over the
object (unless you turn off the pickup option (see below), or move  with  the
`m'  prefix  (see  above)),  or manually by using the `,' command.  If you're
carrying too many things, NetHack will tell you so and won't pick up anything
more.    Otherwise,  it will add the object(s) to your pack and tell you what
you just picked up.

    When you pick up an object, it is assigned an  inventory  letter.    Many
commands  that  operate  on objects must ask you to find out which object you
want to use.  When NetHack asks you to choose a  particular  object  you  are
carrying,  you  are  usually  presented  with  a list of inventory letters to
choose from (see Commands, above).

    Some objects, such as weapons, are easily differentiated.   Others,  like
scrolls  and  potions,  are  given descriptions which vary according to type.
During a game, any two objects with the same description are the  same  type.
However, the descriptions will vary from game to game.

    When you use one of these objects, if its effect is obvious, NetHack will
remember what it is for you.  If its effect isn't extremely obvious, you will
be  asked  what you want to call this type of object so you will recognize it
later.  You can also use the ``#name'' command for the same  purpose  at  any
time, to name all objects of a particular type or just an individual object.

*** 7.1.  Curses and blessings

    Any  object  that you find may be cursed, even if the object is otherwise
helpful.  The most common effect of a curse is being stuck with (and to)  the
item.    Cursed  weapons  weld  themselves  to your hand when wielded, so you
cannot unwield them.  Any cursed item you wear is not removable  by  ordinary
means.    In  addition,  cursed  arms and armor usually, but not always, bear
negative enchantments that make them less effective in combat.  Other  cursed
objects may act poorly or detrimentally in other ways.

    Objects  can  also  become blessed.  Blessed items usually work better or
more beneficially than normal uncursed items.  For example, a blessed  weapon
will do more damage against demons.

    There  are magical means of bestowing or removing curses upon objects, so
even if you are stuck with one, you can still have the curse lifted  and  the
item  removed.   Priests and Priestesses have an innate sensitivity to curses
and blessings, so they can  more  easily  avoid  cursed  objects  than  other
character classes.

    An  item  with  unknown  curse  status,  and an item which you know to be
uncursed, will be distinguished in your inventory by the presence of the word
``uncursed''  in  the  description  of  the latter.  The exception is if this
description isn't needed; you can look at the inventory description and  know
that  you  have  discovered whether it's cursed.  This applies to items which
have ``plusses,'' and items with charges.

*** 7.2.  Weapons (`)')

    Given a  chance,  almost  all  monsters  in  the  Mazes  of  Menace  will
gratuitously  kill  you.    You  need  weapons for self-defense (killing them
first).  Without a weapon, you  do  only  1-2  hit  points  of  damage  (plus
bonuses, if any).

    There  are  wielded  weapons,  like maces and swords, and thrown weapons,
like arrows.  To hit monsters with a weapon, you must  wield  it  and  attack
them,  or  throw  it at them.  To shoot an arrow out of a bow, you must first
wield the bow, then throw the arrow.  Crossbows shoot crossbow bolts.  Slings
hurl  rocks  and  (other) gems.  You can wield only one weapon at a time, but
you can change weapons unless you're wielding a cursed one.

    Enchanted weapons have a ``plus'' (which can also be a minus)  that  adds
to  your  chance  to hit and the damage you do to a monster.  The only way to
find out if a weapon is enchanted is to have it magically identified somehow.

    Those of you in the audience who are AD&D players,  be  aware  that  each
weapon  which  exists  in  AD&D  does the same damage to monsters in NetHack.
Some of the more obscure weapons (such  as  the  aklys,  lucern  hammer,  and
bec-de-corbin)  are  defined  in  an  appendix  to  Unearthed Arcana, an AD&D
supplement.

    The commands to use weapons are `w' (wield) and `t' (throw).

*** 7.3.  Armor (`[')

    Lots of unfriendly things lurk about; you need armor to protect  yourself
from  their  blows.  Some types of armor offer better protection than others.
Your armor class is a measure of  this  protection.    Armor  class  (AC)  is
measured  as  in  AD&D,  with  10 being the equivalent of no armor, and lower
numbers meaning better armor.  Each suit of armor which exists in AD&D  gives
the  same  protection  in NetHack.  Here is an (incomplete) list of the armor
classes provided by various suits of armor:
    dragon scale mail       1
    plate mail              3
    bronze plate mail       4
    splint mail             4
    banded mail             4
    elven mithril-coat      5
    chain mail              5
    scale mail              6
    ring mail               7
    studded leather armor   7
    leather armor           8
    no armor                10

    You can also wear other pieces of armor  (ex.  helmets,  boots,  shields,
cloaks)  to  lower  your  armor class even further, but you can only wear one
item of each category (one suit of armor, one cloak, one helmet, one  shield,
and so on).

    If a piece of armor is enchanted, its armor protection will be better (or
worse) than normal, and its ``plus'' (or minus) will subtract from your armor
class.    For  example, a +1 chain mail would give you better protection than
normal chain mail, lowering your armor class one unit further to 4.  When you
put  on  a  piece  of armor, you immediately find out the armor class and any
``plusses'' it provides.   Cursed  pieces  of  armor  usually  have  negative
enchantments (minuses) in addition to being unremovable.

    The commands to use armor are `W' (wear) and `T' (take off).

*** 7.4.  Food (`%')

    Food is necessary to survive.  If you go too long without eating you will
faint, and eventually die of starvation.   Unprotected  food  does  not  stay
fresh  indefinitely;  after  a  while it will spoil, and be unhealthy to eat.
Food stored in ice boxes or tins (``cans'' to  you  Americans)  will  usually
stay fresh, but ice boxes are heavy, and tins take a while to open.

    When  you  kill  monsters,  they  usually  leave  corpses  which are also
``food.''  Many, but not all, of these are edible; some also give you special
powers when you eat them.  A good rule of thumb is ``you are what you eat.''

    You can name one food item after something you like to eat with the fruit
option, if your dungeon has it.

    The command to eat food is `e'.

*** 7.5.  Scrolls (`?')

    Scrolls are labeled with  various  titles,  probably  chosen  by  ancient
wizards  for  their  amusement  value  (ex.  ``READ  ME,''  or ``HOLY BIBLE''
backwards).  Scrolls disappear after you read them (except  for  blank  ones,
without magic spells on them).

    One  of  the most useful of these is the scroll of identify, which can be
used to determine what another object is, whether it is  cursed  or  blessed,
and  how  many  uses  it  has  left.   Some objects of subtle enchantment are
difficult to identify without these.

    If you receive mail while you are playing (on versions compiled with this
feature), a mail daemon may run up and deliver it to you as a scroll of mail.
To use this feature, you must let NetHack know where to look for new mail  by
setting  the  ``MAIL'' environment variable to the file name of your mailbox.
You may also want to set the ``MAILREADER'' environment variable to the  file
name  of  your  favorite reader, so NetHack can shell to it when you read the
scroll.

    The command to read a scroll is `r'.

*** 7.6.  Potions (`!')

    Potions are distinguished by the color of the liquid  inside  the  flask.
They disappear after you quaff them.

    Clear  potions  are  potions  of  water.   Sometimes these are blessed or
cursed, resulting in holy or unholy water.  Holy water is  the  bane  of  the
undead,  so  potions of holy water are good thing to throw (`t') at them.  It
also is very useful when you dip (``#dip'') other objects in it.

    The command to drink a potion is `q' (quaff).

*** 7.7.  Wands (`/')

    Magic wands have multiple magical charges.  Some wands  are  directional,
you  must give a direction to zap them in.  You can also zap them at yourself
(just give a `.'  or `s' for the direction), but it is often unwise.    Other
wands  are  nondirectional,  they  don't  ask  for directions.  The number of
charges in a wand is random, and decreases by one whenever you use it.

    The command to use a wand is `z' (zap).

*** 7.8.  Rings (`=')

    Rings are very useful items, since they are relatively  permanent  magic,
unlike the usually fleeting effects of potions, scrolls, and wands.

    Putting  on a ring activates its magic.  You can wear only two rings, one
on each ring finger.

    Most rings also cause you to grow hungry more rapidly, the  rate  varying
with the type of ring.

    The commands to use rings are `P' (put on) and `R' (remove).

*** 7.9.  Spell books (`+')

    Spell  books are tomes of mighty magic.  When studied with the `r' (read)
command, they bestow the knowledge of a spell, unless the attempt  backfires.
Reading  a cursed spell book, or one with mystic runes beyond your ken can be
harmful to your health!

    A spell can also backfire when you cast it.  If you  attempt  to  cast  a
spell  well  above your experience level, or cast it at a time when your luck
is particularly bad, you can end up wasting both  the  energy  and  the  time
required in casting.

    Casting  a  spell calls forth magical energies and focuses them with your
naked mind.  Releasing the magical energy releases some of your memory of the
spell  with  it.    Each time you cast a spell, your familiarity with it will
dwindle, until you eventually forget the details completely and must  relearn
it.

    The  command to read a spell book is the same as for scrolls, `r' (read).
The `+' command lists your current spells and the number of spell points they
require.  The `Z' (cast) command casts a spell.

*** 7.10.  Tools (`(')

    Tools  are  miscellaneous  objects with various purposes.  Some tools are
like wands in that they have a limited number of uses.   For  example,  lamps
burn  out  after  a  while.  Other tools are containers, which objects can be
placed into or taken out of.

    The command to use tools is `a' (apply).

*** 7.10.1.  Chests and boxes

    You may encounter chests or boxes in your travels.  These can  be  opened
with  the  ``#loot'' extended command when they are on the floor, or with the
`a' (apply) command when you are carrying one.   However,  chests  are  often
locked,  and require you to either use a key to unlock it, a tool to pick the
lock, or to break it open with brute force.  Chests are unwieldy objects, and
must be set down to be unlocked (by kicking them, using a key or lock picking
tool with the `a' (apply) command, or by using a weapon  to  force  the  lock
with the ``#force'' extended command).

    Some  chests  are trapped, causing nasty things to happen when you unlock
or open them.  You can check  for  and  try  to  deactivate  traps  with  the
``#untrap'' extended command.

*** 7.11.  Amulets (`"')

    Amulets  are very similar to rings, and often more powerful.  Like rings,
amulets have various magical properties, some beneficial, some harmful, which
are activated by putting them on.

    The  commands  to use amulets are the same as for rings, `P' (put on) and
`R' (remove).

*** 7.12.  Gems (`*')

    Some gems are valuable, and can  be  sold  for  a  lot  of  gold  pieces.
Valuable  gems  increase your score if you bring them with you when you exit.
Other small rocks are also categorized  as  gems,  but  they  are  much  less
valuable.

*** 7.13.  Large rocks (``')

    Statues  and  boulders  are  not  particularly  useful, and are generally
heavy.  It is rumored that some statues are not what they seem.

*** 7.14.  Gold (`$')

    Gold adds to your score, and you can buy things in shops with it.    Your
version of NetHack may display how much gold you have on the status line.  If
not, the `$' command will count it.

*** 8.  Options

    Due to variations in personal  tastes  and  conceptions  of  how  NetHack
should  do  things,  there  are  options  you  can  set to change how NetHack
behaves.

*** 8.1.  Setting the options

    There are two ways to set the options.  The first is with the `O' command
in NetHack; the second is with the ``NETHACKOPTIONS'' environment variable.

*** 8.2.  Using the NETHACKOPTIONS environment variable

    The  NETHACKOPTIONS  variable is a comma-separated list of initial values
for the various options.  Some can only be turned on or off.  You turn one of
these  on  by  adding  the name of the option to the list, and turn it off by
typing a `!' or ``no'' before the name.  Others take a character string as  a
value.    You  can set string options by typing the option name, a colon, and
then the value of the string.  The value is terminated by the next  comma  or
the end of string.

    For  example, to set up an environment variable so that ``female'' is on,
``pickup'' is off, the name is set to ``Blue Meanie'', and the fruit  is  set
to ``papaya'', you would enter the command:

    % setenv NETHACKOPTIONS "female,!pickup,name:Blue Meanie,fruit:papaya"

    in csh, or

    $ NETHACKOPTIONS="female,!pickup,name:Blue Meanie,fruit:papaya"
    $ export NETHACKOPTIONS

    in sh or ksh.

*** 8.3.  Customization options

    Here  are explanations of what the various options do.  Character strings
longer than fifty characters are truncated.  Some of the options  listed  may
be inactive in your dungeon.

catname
        Name your starting cat (ex. ``catname:Morris'').  Cannot be set  with
        the `O' command.

color
        Use color for  different  monsters,  objects,  and  dungeon  features
        (default on).

confirm
        Have user confirm attacks on pets, shopkeepers, and  other  peaceable
        creatures (default on).

DECgraphics
        Use a predefined selection  of  characters  from  the  DEC  VTxxx/DEC
        Rainbow/  ANSI  line-drawing  character  set  to  display the dungeon
        instead of having to define a full  graphics  set  yourself  (default
        off).  Cannot be set with the `O' command.

dogname
        Name your starting dog (ex.  ``dogname:Fang'').  Cannot be  set  with
        the `O' command.

endgame
        Control what parts of the score list you are shown at the end
        (ex. ``endgame:5  top  scores/4  around  my score/own  scores'').
        Only the first  letter  of  each  category  (`t',  `a',  or  `o')  is
        necessary.

female
        Set your sex (default off).  Cannot be set with the `O' command.

fixinvlet
        An  object's inventory letter sticks to it when it's dropped (default
        on).  If this is off, dropping an object  shifts  all  the  remaining
        inventory letters.

fruit
        Name a fruit after something you enjoy eating (ex.   ``fruit:mango'')
        (default  ``slime  mold''.  Basically a nostalgic whimsy that NetHack
        uses from time to time.  You should set this to  something  you  find
        more  appetizing  than  slime mold.  Apples, oranges, pears, bananas,
        and melons already exist in NetHack, so don't use those.

graphics
        Set the graphics symbols for screen displays (default
        ``|--------|||-\\/.-|+.#<>^"}{#\\_<>##'').  The graphics  option  (if
        used)  should  come last, followed by a string of up to 35 characters
        to be used instead  of  the  default  map-drawing  characters.    The
        dungeon  map  will  use  the  characters  you  specify instead of the
        default symbols.

        The DECgraphics and IBMgraphics options use predefined selections  of
        graphics  symbols,  so you need not go to the trouble of setting up a
        full graphics string for these common cases.

        Note that this option string is now escape-processed in  conventional
        C  fashion.    This  means that `\' is a prefix to take the following
        character literally, and not as a  special  prefix.    Your  graphics
        strings for NetHack 2.2 and older versions may contain a `\'; it must
        be doubled for the same effect now.  The  special  escape  form  `\m'
        switches  on  the  meta  bit  in the following character, and the `^'
        prefix causes the following character to  be  treated  as  a  control
        character  (so any `^' in your old graphics strings should be changed
        to `\^' now).

        The order of the symbols is:  solid rock, vertical  wall,  horizontal
        wall, upper left corner, upper right corner, lower left corner, lower
        right corner, cross wall, upward T wall, downward T wall, leftward  T
        wall,  rightward  T wall, vertical beam, horizontal beam, left slant,
        right slant, no door,  vertical  open  door,  horizontal  open  door,
        closed door, floor of a room, corridor, stairs up, stairs down, trap,
        web, pool or moat, fountain, kitchen sink, throne, altar, ladder  up,
        ladder  down,  vertical drawbridge, horizontal drawbridge.  You might
        want to use `+' for the corners and T  walls  for  a  more  esthetic,
        boxier  display.    Note that in the next release, new symbols may be
        added, or the present ones rearranged.

        Cannot be set with the `O' command.

help
        If more information is available for an object looked at with the `/'
        command, ask if you want to see it (default  on).  Turning  help  off
        makes  just  looking  at  things faster, since you aren't interrupted
        with the ``More info?'' prompt, but it also means that you might miss
        some interesting and/or important information.

IBM_BIOS
        Use BIOS calls to update the screen display quickly and to  read  the
        keyboard (allowing the use of arrow keys to move) on machines with an
        IBM PC compatible BIOS ROM (default off, PC and ST NetHack only).

IBMgraphics
        Use  a  predefined  selection  of  IBM  extended  ASCII characters to
        display the dungeon instead of having to define a full  graphics  set
        yourself (default off).  Cannot be set with the `O' command.

ignintr
        Ignore interrupt signals, including breaks (default off).

male
        Set your sex (default on, most hackers are male).  Cannot be set with
        the `O' command.

name
        Set your character's name (defaults to your user name).  You can also
        set your character class by appending a dash and the first letter  of
        the  character  class (that is, by suffixing one of -A -B -C -E -H -K
        -P -R -S -T -V -W).  Cannot be set with the `O' command.

news
        Read  the NetHack news file, if present (default on).  Since the news
        is shown at the beginning of the game, there's no  point  in  setting
        this with the `O' command.

number_pad
        Use the number keys to move instead of [yuhjklbn] (default off).

null
        Send padding nulls to the terminal (default off).

packorder
        Specify   the   order   to   list   object    types    in    (default
        ``\")[%?+/=!(*'0_'').    The  value of this option should be a string
        containing the symbols for the various object types.

pickup
        Pick up things you move onto by default (default on).

rawio
        Force raw (non-cbreak) mode for faster output  and  more  bulletproof
        input  (MS-DOS  sometimes treats `^P' as a printer toggle without it)
        (default off).  Note:  DEC  Rainbows  hang  if  this  is  turned  on.
        Cannot be set with the `O' command.

rest_on_space
        Make the space bar a synonym for  the  `.'  (rest)  command  (default
        off).

safe_pet
        Prevent you from (knowingly) attacking your pets (default on).

silent
        Suppress terminal beeps (default on).

sortpack
        Sort the pack contents by type  when  displaying  inventory  (default
        on).

standout
        Boldface monsters and ``--More--'' (default off).

time
        Show the elapsed game time in turns on bottom line (default off).

tombstone
        Draw a tombstone graphic upon your death (default on).

verbose
        Provide more commentary during the game (default on).

    In  some  versions, options may be set in a configuration file on disk as
well as from NETHACKOPTIONS.

*** 9.  Scoring

    NetHack maintains a list of the top scores or scorers  on  your  machine,
depending  on  how  it  is  set  up.  In the latter case, each account on the
machine can post only one non-winning score on  this  list.    If  you  score
higher  than  someone  else  on this list, or better your previous score, you
will be inserted in the proper place under  your  current  name.    How  many
scores are kept can also be set up when NetHack is compiled.

    Your score is chiefly based upon how much experience you gained, how much
loot you accumulated, how deep you explored, and how the game ended.  If  you
quit the game, you escape with all of your gold intact.  If, however, you get
killed in the Mazes of Menace, the guild will only hear  about  90%  of  your
gold  when  your corpse is discovered (adventurers have been known to collect
finder's fees).  So, consider whether you want to take one last hit  at  that
monster  and  possibly live, or quit and stop with whatever you have.  If you
quit, you keep all your gold, but if you swing and live, you might find more.

    If you just want to see what the current top players/games list  is,  you
can type nethack -s all.

*** 10.  Explore mode

    NetHack  is  an  intricate  and  difficult game.  Novices might falter in
fear, aware of their ignorance of the means to  survive.    Well,  fear  not.
Your dungeon may come equipped with an ``explore'' or ``discovery'' mode that
enables you to keep old save files and cheat death, at the paltry cost of not
getting on the high score list.

    There  are  two  ways of enabling explore mode.  One is to start the game
with the -X switch.  The other is to issue  the  `X'  command  while  already
playing the game.  The other benefits of explore mode are left for the trepid
reader to discover.

*** 11.  Credits

    The original hack game was modeled  on  the  Berkeley  UNIX  rogue  game.
Large  portions  of  this  paper were shamelessly cribbed from A Guide to the
Dungeons of Doom, by Michael C. Toy and Kenneth  C.  R.  C.  Arnold.    Small
portions  were  adapted  from Further Exploration of the Dungeons of Doom, by
Ken Arromdee.

    NetHack is the product of literally dozens of people's work.  Main events
in the course of the game development are described below:

    Jay Fenlason wrote the original Hack, with help from Kenny Woodland, Mike
Thome and Jon Payne.

    Andries Brouwer did a major  re-write,  transforming  Hack  into  a  very
different  game,  and  published (at least) three versions (1.0.1, 1.0.2, and
1.0.3) for UNIX machines to the Usenet.

    Don G. Kneller ported Hack 1.0.3 to Microsoft C and MS-DOS, producing  PC
HACK 1.01e, added support for DEC Rainbow graphics in version 1.03g, and went
on to produce at least four more versions (3.0, 3.2, 3.51, and 3.6).

    R. Black ported PC HACK 3.51 to  Lattice  C  and  the  Atari  520/1040ST,
producing ST Hack 1.03.

    Mike   Stephenson   merged   these   various   versions   back  together,
incorporating many of the added features, and produced NetHack 1.4.  He  then
coordinated  a  cast  of thousands in enhancing and debugging NetHack 1.4 and
released NetHack versions 2.2 and 2.3.

    Later, Mike coordinated a major rewrite of the game, heading a team which
included Ken Arromdee, Jean-Christophe Collet, Steve Creps, Eric Hendrickson,
Izchak Miller, John Rupley, Mike  Threepoint,  and  Janet  Walz,  to  produce
NetHack 3.0c.

    NetHack  3.0  was  ported  to the Atari by Eric R. Smith, to OS/2 by Timo
Hakulinen, and to VMS by David Gentzel.  The three of them  and  Kevin  Darcy
later  joined  the  main  development team to produce subsequent revisions of
3.0.

    Olaf Seibert ported NetHack 2.3 and 3.0  to  the  Amiga.    Norm  Meluch,
Stephen  Spackman  and  Pierre Martineau designed overlay code for PC NetHack
3.0.  Johnny Lee ported NetHack 3.0 to the Macintosh.    Along  with  various
other  Dungeoneers,  they  continued  to enhance the PC, Macintosh, and Amiga
ports through the later revisions of 3.0.

    From time to time, some depraved individual out there in netland sends  a
particularly  intriguing modification to help out with the game.  The Gods of
the Dungeon sometimes make note of the names of the worst of these miscreants
in this, the list of Dungeoneers:
Richard Addison           Bruce Holloway            Pat Rankin
Tom Almy                  Richard P. Hughey         Eric S. Raymond
Ken Arromdee              Ari Huttunen              John Rupley
Eric Backus               Del Lamb                  Olaf Seibert
John S. Bien              Greg Laskin               Kevin Sitze
Ralf Brown                Johnny Lee                Eric R. Smith
Jean-Christophe Collet    Steve Linhart             Kevin Smolkowski
Steve Creps               Ken Lorber                Michael Sokolov
Kevin Darcy               Benson I. Margulies       Stephen Spackman
Matthew Day               Pierre Martineau          Andy Swanson
Joshua Delahunty          Roland McGrath            Kevin Sweet
Jochen Erwied             Norm Meluch               Scott R. Turner
David Gentzel             Bruce Mewborne            Janet Walz
Mark Gooderum             Izchak Miller             Jon Watte
David Hairston            Gil Neiger                Tom West
Timo Hakulinen            Greg Olson                Gregg Wonderly
Eric Hendrickson          Mike Passaretti

    Brand  and product names are trademarks or registered trademarks of their
respective holders.
