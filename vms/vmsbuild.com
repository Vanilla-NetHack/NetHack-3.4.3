$ makedefs := $sys$disk:[]makedefs
$ cc = "CC/NOLIST/OPT=NOINLINE/DEB/INCL=[-.INCLUDE]/DEFI=(""WIZARD=""""GENTZEL"""""")"
$ link := link/nomap'p2'
$ if p1 .eqs. "LINK" then goto link
$ define sys sys$library:
$ cc alloc.c
$ cc makedefs.c
$ cc monst.c
$ cc objects.c
$ cc panic.c
$ cc [-.vms]vmsmisc
$ link makedefs.obj,monst.obj,objects.obj,panic.obj,vmsmisc.obj,sys$input:/opt
sys$share:vaxcrtl/share
$ makedefs -p
$ makedefs -o
$ makedefs -t
$ cc allmain.c
$ cc apply.c
$ cc artifact.c
$ cc attrib.c
$ cc bones.c
$ cc cmd.c
$ cc dbridge.c
$ cc decl.c
$ cc demon.c
$ cc do.c
$ cc do_name.c
$ cc do_wear.c
$ cc dog.c
$ cc dogmove.c
$ cc dokick.c
$ cc dothrow.c
$ cc eat.c
$ cc end.c
$ cc engrave.c
$ cc exper.c
$ cc extralev.c
$ cc fountain.c
$ cc getline.c
$ cc hack.c
$ cc invent.c
$ cc lock.c
$ cc mail.c
$ cc [-.vms]vmsmain.c/obj=main.obj
$ cc makemon.c
$ cc mcastu.c
$ cc mhitm.c
$ cc mhitu.c
$ cc mklev.c
$ cc mkmaze.c
$ cc mkobj.c
$ cc mkroom.c
$ cc mon.c
$ cc mondata.c
$ cc monmove.c
$ cc mthrowu.c
$ cc music.c
$ cc o_init.c
$ cc objnam.c
$ cc options.c
$ cc pager.c
$ cc pickup.c
$ cc polyself.c
$ cc potion.c
$ cc pray.c
$ cc pri.c
$ cc priest.c
$ cc prisym.c
$ cc read.c
$ cc restore.c
$ cc rip.c
$ cc rnd.c
$ cc rumors.c
$ cc save.c
$ cc search.c
$ cc shk.c
$ cc shknam.c
$ cc sit.c
$ cc sounds.c
$ cc sp_lev.c
$ cc spell.c
$ cc steal.c
$ cc termcap.c
$ cc timeout.c
$ cc topl.c
$ cc topten.c
$ cc track.c
$ cc trap.c
$ cc [-.vms]vmstty.c/obj=tty.obj
$ cc u_init.c
$ cc uhitm.c
$ cc [-.vms]vmsunix.c/obj=unix.obj
$ cc vault.c
$ makedefs -v
$ cc version.c
$ cc weapon.c
$ cc were.c
$ cc wield.c
$ cc wizard.c
$ cc worm.c
$ cc worn.c
$ cc write.c
$ cc zap.c
$ cc [-.others]random.c
$ cc/def="bcopy(s1,s2,sz)=memcpy(s2,s1,sz)" [-.vms]vmstermcap.c
$ cc [-.vms]vmstparam.c
$link:
$ link/exe=nethack sys$input:/opt
allmain.obj,-
alloc.obj,-
apply.obj,-
artifact.obj,-
attrib.obj,-
bones.obj,-
cmd.obj,-
dbridge.obj,-
decl.obj,-
demon.obj,-
do.obj,-
do_name.obj,-
do_wear.obj,-
dog.obj,-
dogmove.obj,-
dokick.obj,-
dothrow.obj,-
eat.obj,-
end.obj,-
engrave.obj,-
exper.obj,-
extralev.obj,-
fountain.obj,-
getline.obj,-
hack.obj,-
invent.obj,-
lock.obj,-
mail.obj,-
main.obj,-
makemon.obj,-
mcastu.obj,-
mhitm.obj,-
mhitu.obj,-
mklev.obj,-
mkmaze.obj,-
mkobj.obj,-
mkroom.obj,-
mon.obj,-
mondata.obj,-
monmove.obj,-
monst.obj,-
mthrowu.obj,-
music.obj,-
o_init.obj,-
objects.obj,-
objnam.obj,-
options.obj,-
pager.obj,-
pickup.obj,-
polyself.obj,-
potion.obj,-
pray.obj,-
pri.obj,-
priest.obj,-
prisym.obj,-
read.obj,-
restore.obj,-
rip.obj,-
rnd.obj,-
rumors.obj,-
save.obj,-
search.obj,-
shk.obj,-
shknam.obj,-
sit.obj,-
sounds.obj,-
sp_lev.obj,-
spell.obj,-
steal.obj,-
termcap.obj,-
timeout.obj,-
topl.obj,-
topten.obj,-
track.obj,-
trap.obj,-
tty.obj,-
u_init.obj,-
uhitm.obj,-
unix.obj,-
vault.obj,-
version.obj,-
weapon.obj,-
were.obj,-
wield.obj,-
wizard.obj,-
worm.obj,-
worn.obj,-
write.obj,-
zap.obj,-
random.obj,-
vmsmisc.obj,-
vmstermcap.obj,-
vmstparam.obj
sys$library:vaxcrtl/library
