#if 0

static void
test_md ()
{
  FILE *fp = NULL;
  int i;
  char buf[0x10000];
  
  for (i = 0; i < 0x10000; ++i)
  {
    fp = fopen ("/md/quick", "r");
    if (!fp)
      dc_put_error ("md open error");
    fclose(fp);
    
    if (!(i % 0x100))
    {
      sprintf(buf, "test_md step1 %x", i);
      dc_print(buf);
    }
  }
  
  for (i = 0; i < 0x1000; ++i)
  {
    fp = fopen("/md/quick", "w");
    if (!fp)
      dc_put_error ("md open error");
    
    if (fwrite(buf, sizeof(buf), 1, fp) != 1)
      dc_put_error ("write error");
    
    fclose(fp);
    
    if (!(i % 0x100))
    {
      sprintf(buf, "test_md step2 %x", i);
      dc_print(buf);
    }
  }
  
  for (i = 0; i < 0x1000; ++i)
  {
    fp = fopen("/md/quick", "r");
    if (!fp)
      dc_put_error ("md open error");
    
    if (fread(buf, sizeof(buf), 1, fp) != 1)
      dc_put_error ("read error");
    
    fclose(fp);
    
    if (!(i % 0x100))
    {
      sprintf(buf, "test_md step3 %x", i);
      dc_print(buf);
    }
  }
}


static void
test_nes ()
{
  int i;
  NES_pad *pad = NULL;
  sound_mgr *snd_mgr = NULL;
  dc_NES_screen_mgr *scr_mgr = NULL;
  char buf[128];
  
  for (i = 0; i < 0x10000; ++i)
  {
    pad = new NES_pad ();
    scr_mgr = new dc_NES_screen_mgr ();
    // snd_mgr = new dc_sound_mgr ();
    
    delete (scr_mgr);
    delete (pad);
    
    if (!(i % 0x100))
    {
      sprintf(buf, "test_nes step1 %x", i);
      dc_print(buf);
    }
  }
}


static void
test_bzip2 ()
{
  char buf[128];
  char buf1[0x4000];
  char buf2[0x4000];
  int buf1_len = sizeof(buf1);
  int buf2_len = sizeof(buf2);
  int i;
  
  for (i = 0; i < sizeof(buf1); ++i)
  {
    if (i & 0x01)
      buf1[i] = buf1[i - 1] >> 1;
    else
      buf1[i] = (i >> 1) ^ 0xedb88320;
    
    if (!(i % 0x100))
    {
      sprintf(buf, "test_bzip2 step0 %x", i);
      dc_print(buf);
    }
  }
  
  for (i = 0; i < 0x10; ++i)
  {
    buf2_len = sizeof(buf2);
    if (BZ2_bzBuffToBuffCompress (buf2, (unsigned int *)&buf2_len, 
				  buf1, sizeof(buf1), 
				  1, 0, 0) != BZ_OK)
    {
      dc_put_error ("compress error");
    }
    
    buf1_len = sizeof(buf1);
    if (BZ2_bzBuffToBuffDecompress (buf1, (unsigned int *)&buf1_len,
				    buf2, buf2_len,
				    0, 0) != BZ_OK)
    {
      dc_put_error ("decompress error");
    }
    
    sprintf(buf, "test_bzip2 step1 %x", i);
    dc_print(buf);
  }
}


static void
test_input ()
{
  uint16 image[320 * 240];
  
  load_bmp (image, "/cd/pics/startup.bmp");
  
  for (;;)
  {
    cont_cond_t cont;
    static int count;
    char str[128];
    
    cont_get_cond(dc_controller_addr[0], &cont);
    
    if (!(cont.buttons & CONT_START))
      break;
    
    ++count;
    display_rawimage (image);
    sprintf(str, "count = %d", count);
    draw_string(10, 210, _white, _black, str);
    sprintf(str, "button = 0x%x; addr = %d", cont.buttons, dc_controller_addr[0]);
    draw_string(10, 200, _white, _black, str);
    dc_vid_flip(draw_type_fullscreen);
  }
}


#endif



static void
key_test()
{
  cont_state_t *stat;
  char str[256];
  char *title = "KEY TEST";
  char *foot = "Press Controller's A";
  
  dc_pvr_font_output_message (title, NULL, foot, NULL);
  for (;;)
  {
    if (dc_maple_keyboard_info->dev)
    {
      int key;
      
      if (dc_maple_keyboard_info->dev)
      {
	kbd_state_t *kstat;
	kbd_cond_t *cond;
	int i;
	
	kstat = (kbd_state_t *)maple_dev_status (dc_maple_keyboard_info->dev);
	cond = &(kstat->cond);
	
	if (cond->modifiers & KBD_MOD_LCTRL) dc_pvr_font_output_message (title, "LCTRL", foot, NULL);
	if (cond->modifiers & KBD_MOD_LSHIFT) dc_pvr_font_output_message (title, "LSHIFT", foot, NULL);
	if (cond->modifiers & KBD_MOD_RSHIFT) dc_pvr_font_output_message (title, "RSHIFT", foot, NULL);
	
	for (i = 0; i < sizeof(cond->keys)/sizeof(cond->keys[0]); ++i)
	{
	  if (cond->keys[i] != 0)
	  {
	    sprintf(str, "%d: 0x%02x => 0x%02x", sizeof(cond->keys)/sizeof(cond->keys[0]), cond->keys[i], keymap_dc2nes[cond->keys[i]]);
	    dc_pvr_font_output_message (title, str, foot, NULL);
	  }
	}
      }
    }
    
    stat = (cont_state_t *)maple_dev_status(dc_maple_controller_info->dev);
    
    if (stat->buttons & CONT_A) return;
  }
}



static void
test_all ()
{
#if 0
  test_md ();
  test_nes ();
  test_bzip2 ();
#endif
  key_test ();
}

