** FM-7 series  emulator for common source code project. **
                                               December 18, 2018
		   K.Ohta <whatisthis.sowhat _at_ gmail.com>

1.Background
  Major FM-7 series emulator, XM7 is closed source code, not FOSS.
  But, I was porting to SDL/Agar toolkit.[1]
  This has many of bugs inheritated from Agar Toolkit.
  And, I wish to distribute FM-7 emulator with FOSS license,
  cause of copyright violation issue by MESS.
  So, I decided to build FM-7 emulator to Common Source Code
  Project [2], this is distributed with GPLv2.

  [1] https://github.com/Artanejp/XM7-for-SDL
  [2] http://takeda-toshiya.my.coocan.jp/
  
2.Status
  a. FM-7 is working now.
  b. FM-77 is working now (but some features are wrong).
     Especially 400 line card and 2HD FDDs are still not implement,
     because I don't have these boards.
  c. FM-8 is implement, but not tested enough.
     I implemented bubble casette, but not complete implement.
     Seems to be working, but not completed.
     But, I still not implement 8"FDD.
  d. FM77AV is mostly working.
  e. FM77AV40/EX is mostly working.
  f. Implemented hidden message of FM77AV's keyboard [3].
  g. Implemented saving/loading state feature, mostly working.
  h. Include printer support without inteligent commands.
     Also inmplement Dempa Shimbun-sha's Joystick (sold with XEVIOUS).
  i. Add also FM77AV's hidden buzzer.
  j. Support connect/disconnect FDDs. Please check "Connect 320KB FDD" at first.
  k. Support auto input "8" or "5" when using ten-key as moving characters at games for FM-8 and FM-7.
  l. Z80 Card (Only with FM-7/77 and FM-8) works.But not tested enough.
  m. Japanese Communication board works for FM-7/NEW7/77/L4.
  n. Start to implement RS-232C/MODEM/MIDI feature.(WIP)
  o. Start to implement FM-77L4's 400 lines board.But imcomplete.(WIP)Still WIP (2018-05-16)
  If you need more informations, see RELEASENOTE and reame_by_artane.txt .
     
  [3] I implemented hidden message faster than XM7 :-)
  
3.How to Work
  You Need these R@M images to work FM-7.
  If you don't have these images, you can get substitution R@Ms
  from : http://retropc.net/apollo/download/xm7/romset/index.htm .
  
  At least for FM-7 or later:
  BOOT_BAS.ROM : 512 bytes, To boot as BASIC mode.
  BOOT_DOS.ROM : 512 bytes, To boot as DOS(NOT MS-DOS) mode.
  FBASIC302.ROM
  FBASIC300.ROM
  FBASIC30.ROM : 31744 bytes, F-BASIC 3.0 code,
                 Dummy (only BIOS) rom if you use substitution ROMS.
  SUBSYS_C.ROM : 10240 bytes, Monitor of SUBCPU.

  Optionally ROMS:
  KANJI.ROM
  KANJI1.ROM   : 131072 bytes, Kanji JIS class 1 patterns.
  BOOT_MMR.ROM : 512 bytes, hidden boot ROM for FM-77 (only).

  Optional ROMS (Only for FM77L4):
  SUBSYS_L4.ROM : 18432 bytes. System ROM of 400 lines board.
  ANKCG16.ROM   : 4096 bytes.  Character data of 400 lines board.
  
   You need belows if you try to work FM77AV:
  INITIATE.ROM : 8192 bytes, initiator ROM.
                 You must use *pure* FM77AV's Initiator ROM.
  SUBSYSCG.ROM : 8192 bytes, character data for subsystem.
  SUBSYS_A.ROM : 8192 bytes, monitor type A for sub system.
  SUBSYS_B.ROM : 8192 bytes, monitor type B for sub system.

  Optionally ROMS (For FM77AV20/40):
  KANJI2.ROM   : 131072 bytes, Kanji JIS class 2 patterns.
  DICROM.ROM   : 262144 bytes, Dictionary data for Kana-Kanji conversion.
  EXTSUB.ROM   : 49152 bytes, extra monitor for subsystem (77AV20 or later?)
  
  If you use emufm77av40ex, you must use FM-77AV40EX/SX's initiator ROM.

  Making if you use DICROM :
  USERDIC.DAT  : 8192 bytes, learning data of Kana-Kanji conversion.


4. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7
      https://osdn.jp/projects/csp-qt/

Enjoy!
-- K.Ohta.
