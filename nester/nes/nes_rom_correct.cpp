
// data base to correct the rom image header renamed by nestoy

// Mapper 000 -----------------------------------------------------------------

if(crc32() == 0x57970078)   // F-1 Race (J)
{
  ROM_banks[0x3FE1] = 0xFF;
  ROM_banks[0x3FE6] = 0x00;
}

if(crc32() == 0xaf2bbcbc)   // Mach Rider (JU)
{
  ROM_banks[0x7FDF] = 0xFF;
  ROM_banks[0x7FE4] = 0x00;
}

if(crc32() == 0xe16bb5fe)   // Zippy Race (J)
{
  header.flags_1 &= 0xf6;
}

// Mapper 001 -----------------------------------------------------------------

if(crc32() == 0x7831b2ff || // America Daitouryou Senkyo (J)
   crc32() == 0x190a3e11 || // Be-Bop-Highschool - Koukousei Gokuraku Densetsu (J)
   crc32() == 0x52449508 || // Home Run Nighter - Pennant League!! (J)
   crc32() == 0x0973f714 || // Jangou (J)
   crc32() == 0x7172f3d4 || // Kabushiki Doujou (J)
   crc32() == 0xa5781280 || // Kujaku Ou 2 (J)
   crc32() == 0x8ce9c87b || // Money Game, The (J)
   crc32() == 0xec47296d || // Morita Kazuo no Shougi (J)
   crc32() == 0xcee5857b || // Ninjara Hoi! (J)
   crc32() == 0xe63d9193 || // Tanigawa Kouji no Shougi Shinan 3 (J)
   crc32() == 0xd54f5da9)   // Tsuppari Wars (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}
if(crc32() == 0xb8747abf)   // Best Play - Pro Yakyuu Special (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 003 -----------------------------------------------------------------

if(crc32() == 0x8218c637)   // Space Hunter (J)
{
  header.flags_1 &= 0xf6;
  header.flags_1 |= MASK_4SCREEN_MIRRORING;
}
if(crc32() == 0x2bb6a0f8 || // Sherlock Holmes - Hakushaku Reijou Yuukai Jiken (J)
   crc32() == 0x28c11d24 || // Sukeban Deka 3 (J)
   crc32() == 0x02863604)   // Sukeban Deka 3 (J) (Alt)
{
  header.flags_1 &= 0xf6;
  header.flags_1 |= MASK_VERTICAL_MIRRORING;
}
if(crc32() == 0x29401686)   // Minna no Taabou no Nakayoshi Dai Sakusen (J)
{
  ROM_banks[0x2B3E] = 0x60;
}
if(crc32() == 0x9b6d2cb5)   // Mickey Mousecapade (J) [a]
{
  ROM_banks[0x5B98] = 0x80;
}
if(crc32() == 0x932a077a)   // TwinBee (J)
{
  mapper = 87;
}

// Mapper 004 -----------------------------------------------------------------

if(crc32() == 0x58581770)   // Rasaaru Ishii no Childs Quest (J)
{
  header.flags_1 &= 0xf6;
  header.flags_1 |= MASK_VERTICAL_MIRRORING;
}
if(crc32() == 0xf3feb3ab || // Kunio Kun no Jidaigeki Dayo Zenin Shuugou! (J)
   crc32() == 0xa524ae9b || // Otaku no Seiza - An Adventure in the Otaku Galaxy (J)
   crc32() == 0x46dc6e57 || // SD Gundam - Gachapon Senshi 2 - Capsule Senki (J)
   crc32() == 0x66b2dec7 || // SD Gundam - Gachapon Senshi 3 - Eiyuu Senki (J)
   crc32() == 0x92b07fd9 || // SD Gundam - Gachapon Senshi 4 - New Type Story (J)
   crc32() == 0x8ee6463a || // SD Gundam - Gachapon Senshi 5 - Battle of Universal Century (J)
   crc32() == 0xaf754426 || // Ultraman Club 3 (J)
   crc32() == 0xfe4e5b11 || // Ushio to Tora - Shinen no Daiyou (J)
   crc32() == 0x57c12c17)   // Yamamura Misa Suspense - Kyouto Zaiteku Satsujin Jiken (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}
if(crc32() == 0x42e03e4a)   // RPG Jinsei Game (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
  mapper = 118;
}

if (crc32() == 0x3b0fb600) // Ys 3 Wanderers From Ys (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
  mapper = 118;
}


// Mapper 005 -----------------------------------------------------------------
if(crc32() == 0xe91548d8)   // Shin 4 Nin Uchi Mahjong - Yakuman Tengoku (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 010 -----------------------------------------------------------------

if(crc32() == 0xc9cce8f2)   // Famicom Wars (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 011 -----------------------------------------------------------------

if(crc32() == 0x6467a5c4)   // Maxi 15 (Unl) (U)
{
  mapper = 234;
}

// Mapper 016 -----------------------------------------------------------------

if(crc32() == 0x983d8175 || // Datach - Battle Rush - Build Up Robot Tournament (J)
   crc32() == 0x894efdbc || // Datach - Crayon Shin Chan - Ora to Poi Poi (J)
   crc32() == 0x19e81461 || // Datach - Dragon Ball Z - Gekitou Tenkaichi Budou Kai (J)
   crc32() == 0xbe06853f || // Datach - J League Super Top Players (J)
   crc32() == 0x0be0a328 || // Datach - SD Gundam - Gundam Wars (J)
   crc32() == 0x5b457641 || // Datach - Ultraman Club - Supokon Fight! (J)
   crc32() == 0xf51a7f46 || // Datach - Yuu Yuu Hakusho - Bakutou Ankoku Bujutsu Kai (J)
   crc32() == 0x31cd9903 || // Dragon Ball Z - Kyoushuu! Saiya Jin (J)
   crc32() == 0xe49fc53e || // Dragon Ball Z 2 - Gekishin Freeza!! (J)
   crc32() == 0x09499f4d || // Dragon Ball Z 3 - Ressen Jinzou Ningen (J)
   crc32() == 0x2e991109)   // Dragon Ball Z Gaiden - Saiya Jin Zetsumetsu Keikaku (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 019 -----------------------------------------------------------------

if(crc32() == 0x3296ff7a || // Battle Fleet (J)
   crc32() == 0x429fd177 || // Famista '90 (J)
   crc32() == 0xdd454208 || // Hydlide 3 - Yami Kara no Houmonsha (J)
   crc32() == 0xb1b9e187 || // Kaijuu Monogatari (J)
   crc32() == 0xaf15338f)   // Mindseeker (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 026 -----------------------------------------------------------------

if(crc32() == 0x836cc1ab)   // Mouryou Senki Madara (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 033 -----------------------------------------------------------------

if(crc32() == 0x547e6cc1)   // Flintstones - The Rescue of Dino & Hoppy, The (J)
{
  mapper = 48;
}
if(crc32() == 0xa71c3452)   // Insector X (J)
{
  header.flags_1 &= 0xf6;
  header.flags_1 |= MASK_VERTICAL_MIRRORING;
}

// Mapper 043 -----------------------------------------------------------------

if(crc32() == 0x6175b9a0)   // 150-in-1 (Pirate Cart)
{
  mapper = 235;
}

// Mapper 047 -----------------------------------------------------------------

if(crc32() == 0x7eef434c)   // Supare Mario Bros, Tetris, Nintendo World Cup
{
  mapper = 47;
}

// Mapper 065 -----------------------------------------------------------------

if(crc32() == 0xfd3fc292)   // Ai Sensei no Oshiete - Watashi no Hoshi (J)
{
  mapper = 32;
}

// Mapper 068 -----------------------------------------------------------------

if(crc32() == 0xfde79681)   // Maharaja (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 080 -----------------------------------------------------------------

if(crc32() == 0x95aaed34 || // Mirai Shinwa Jarvas (J)
   crc32() == 0x17627d4b)   // Taito Grand Prix - Eikou heno License (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 082 -----------------------------------------------------------------

if(crc32() == 0x4819a595)   // Kyuukyoku Harikiri Stadium - Heisei Gannen Ban (J)
{
  header.flags_1 |= MASK_HAS_SAVE_RAM;
}

// Mapper 086 -----------------------------------------------------------------

if(crc32() == 0xe63f7d0b)   // Urusei Yatsura - Lum no Wedding Bell (J)
{
  mapper = 101;
}

// VS Unisystem ---------------------------------------------------------------

if(crc32() == 0xeb2dba63)   // VS TKO Boxing
{
  mapper = 4;
}

if(crc32() == 0xcbe85490 || // VS Excitebike
   crc32() == 0x29155e0c || // VS Excitebike (Alt)
   crc32() == 0x17ae56be || // VS Freedom Force
   crc32() == 0xff5135a3)   // VS Hogan's Alley
{
  header.flags_1 &= 0xf6;
  header.flags_1 |= MASK_4SCREEN_MIRRORING;
}

if(crc32() == 0x0b65a917)   // VS Mach Rider
{
  ROM_banks[0x7FDF] = 0xFF;
  ROM_banks[0x7FE4] = 0x00;
}


// PAL roms -------------------------------------------------------------------

if (crc32 () == 0xd161888b || // kick off
    crc32 () == 0x637e366a) // castlevania 3
{
  monitor_type = monitor_pal;
}
