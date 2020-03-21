
// data base to set the expand controller automaticaly

ex_controller_type = EX_NONE;

// Crazy Climber Controller ---------------------------------------------------

if(crc32() == 0xc68363f6 ||  // Crazy Climber (J)
   crc32() == 0xc1dc5b12)    // Crazy Climber
{
  ex_controller_type = EX_CRAZY_CLIMBER;
}

// Family Basic Keyboard with Data Recorder -----------------------------------

if(                         // Family BASIC (Ver 1.0)
   crc32() == 0xf9def527 || // Family BASIC (Ver 2.0)
   crc32() == 0xde34526e || // Family BASIC (Ver 2.1a)
   crc32() == 0xf050b611 || // Family BASIC (Ver 3)
   crc32() == 0x3aaeed3f    // Family BASIC (Ver 3) (Alt)
                        )   // Play Box Basic
{
  ex_controller_type = EX_FAMILY_KEYBOARD;
}

// Hyper Shot -----------------------------------------------------------------

if(crc32() == 0xff6621ce || // Hyper Olympic (J)
   crc32() == 0xdb9418e8 || // Hyper Olympic (Tonosama Ban) (J)
   crc32() == 0xac98cd70)   // Hyper Sports (J)
{
  ex_controller_type = EX_HYPER_SHOT;
}

// Optical Gun (Zapper) -------------------------------------------------------

if(crc32() == 0xfbfc6a6c || // Adventures of Bayou Billy, The (E)
   crc32() == 0xcb275051 || // Adventures of Bayou Billy, The (U)
   crc32() == 0xfb69c131 || // Baby Boomer (Unl) (U)
   crc32() == 0xf2641ad0 || // Barker Bill's Trick Shooting (U)
   crc32() == 0xbc1dce96 || // Chiller (Unl) (U)
   crc32() == 0x90ca616d || // Duck Hunt (JUE)
   crc32() == 0x59e3343f || // Freedom Force (U)
   crc32() == 0x242a270c || // Gotcha! (U)
   crc32() == 0x7b5bd2de || // Gumshoe (UE)
   crc32() == 0x255b129c || // Gun Sight (J)
   crc32() == 0x8963ae6e || // Hogan's Alley (JU)
   crc32() == 0x51d2112f || // Laser Invasion (U)
   crc32() == 0x0a866c94 || // Lone Ranger, The (U)
   crc32() == 0xe4c04eea || // Mad City (J)
   crc32() == 0x9eef47aa || // Mechanized Attack (U)
   crc32() == 0xc2db7551 || // Shooting Range (U)
   crc32() == 0x163e86c0 || // To The Earth (U)
   crc32() == 0x389960db)   // Wild Gunman (JUE)
{
  ex_controller_type = EX_OPTICAL_GUN;
}

// Space Shadow Gun (Hyper Shot) ----------------------------------------------

if(crc32() == 0x0cd00488)   // Space Shadow (J)
{
  ex_controller_type = EX_SPACE_SHADOW_GUN;
}

