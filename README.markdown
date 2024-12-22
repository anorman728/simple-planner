# Simple Planner

Every calendar software that I've ever used, ever, has been terrible.  Not just recent ones, but when I was using Palm OS and even those digital organizers in the 90s.

The reason they're terrible is that they insist that every item either has a *time* or be an "all day event".  And what's a "Task" vs "Event"?  I just want to put an item on a calendar like I would with a regular paper calendar.

So, I made this, which is literally just adding items to days.  The only additional feature is that something can be repeated yearly.

It's CLI, but it works fine in Termux on Android.

`make release` will build it in Termux and probably most Linux-based systems.  It should work in WSL, too, I'd imagine.  It might work with Windows with w64devkit, but I don't know how you'd link sqlite3.  (I'm sure you can.  I just don't know how.)

When I describe how to use this, it might seem overwhelming, but it's actually pretty easy to pick up.  It's *almost* self-explanatory after it's been compiled.  You launch the program with `/path/to/exe /path/to/dbfile`, e.g. `./simple-planner planner.db`.

(As a quick sidenote, I created a short bash script named 'p' and put that in one of the directories in $PATH, so I only have to type `p`, then hit enter, and I'm in the program.)

Once it's started, you'll see a screen that looks like this:

```
S 2024-12-22

M 2024-12-23

T 2024-12-24

W 2024-12-25

R 2024-12-26

F 2024-12-27

A 2024-12-28

(A)dd, (E)dit, (D)elete, (P)revious, (N)ext, (C)urrent, (T)oday, (G)oto, (Q)uit
>
```

Here, it waits for input, which can be any of the parenthesized letters on that last line.  If you do it wrong (which you will the first time if you're not clairvoyent), it'll tell you what you did wrong.  Usually.

Just as an example, if I wanted to add something to Thursday, I'd type 'A R' (or 'a r', because it's not case-sensitive), then enter.  Then it'll ask me for a description and whether or not it's yearly.  Then it'll show this.

```
S 2024-12-22

M 2024-12-23

T 2024-12-24

W 2024-12-25

R 2024-12-26
  1) new item on thursday

F 2024-12-27

A 2024-12-28

(A)dd, (E)dit, (D)elete, (P)revious, (N)ext, (C)urrent, (T)oday, (G)oto, (Q)uit
>
```

Now that new item has a number, so if I want to edit it I'd do it with 'e 1', and if I wanted to delete it, I'd do it with 'd 1'.

"Current" means redisplay the week that was most recently displayed.  "Today" means go to the week that includes right this moment.

The only other one that requires explanation is Goto-- It needs to be in the format YYMMDD, so if I want to go to the week that includes July 6, 2027, I'd type "g 270706".
