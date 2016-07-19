
namespace BitEngine {

	struct ERROR_TEXTURE_DATA {
		int 	 width;
		int 	 height;
		unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
		unsigned char	 pixel_data[32 * 32 * 3 + 1];
	};

	ERROR_TEXTURE_DATA error_texture_data = {
		32, 32, 3,
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\376\204\204"
		"\376nn\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\376\227\227\376\244\244\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\375rr\373"
		"\0\0\373\0\0\375hh\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\375\235"
		"\235\374\0\0\374\0\0\375\235\235\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\373ee\371\0\0\372\0\0\372\0"
		"\0\371\0\0\373ee\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\375\236\236\372\0\0\372\0\0\372"
		"\0\0\372\0\0\374\222\222\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\372[[\367\0\0\370\0\0\370\0\0\370\0\0\370\0\0\367\0\0\373cc"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\374\236\236\367\0\0\370\0\0\370\0\0\370\0\0\370\0\0\370\0\0\373\202\202"
		"\377\377\377\377\377\377\377\377\377\370QQ\365\0\0\366\0\0\366\0\0\366\0"
		"\0\366\0\0\366\0\0\366\0\0\365\0\0\372aa\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\373\240\240\365\0\0\366\0\0\366\0\0\366\0\0\366\0\0\366"
		"\0\0\366\0\0\365\0\0\372qq\377\377\377\367NN\363\0\0\364\0\0\364\0\0\364"
		"\0\0\364\0\0\364\0\0\364\0\0\364\0\0\364\0\0\363\0\0\370^^\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\372\241\241\364\0\0\364\0\0\364\0\0\364\0\0\364\0\0\364\0\0\364"
		"\0\0\364\0\0\364\0\0\363\0\0\371ff\373\264\264\361\0\0\361\0\0\361\0\0\361"
		"\0\0\361\0\0\361\0\0\361\0\0\361\0\0\361\0\0\361\0\0\361\0\0\365[[\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\372\242"
		"\242\361\0\0\361\0\0\361\0\0\361\0\0\361\0\0\361\0\0\361\0\0\361\0\0\361"
		"\0\0\361\0\0\361\0\0\371\233\233\377\377\377\372\262\262\357\0\0\360\0\0"
		"\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\357\0\0"
		"\364WW\377\377\377\377\377\377\377\377\377\377\377\377\371\243\243\360\0"
		"\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0\0\360\0"
		"\0\357\0\0\371\232\232\377\377\377\377\377\377\377\377\377\371\257\257\355"
		"\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356"
		"\0\0\356\0\0\363TT\377\377\377\377\377\377\370\245\245\356\0\0\355\0\0\356"
		"\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356\0\0\356\0\0\355\0\0\370"
		"\231\231\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\370"
		"\254\254\353\0\0\354\0\0\354\0\0\354\0\0\354\0\0\354\0\0\354\0\0\354\0\0"
		"\354\0\0\354\0\0\353\0\0\362SS\370\251\251\354\0\0\353\0\0\354\0\0\354\0"
		"\0\354\0\0\354\0\0\354\0\0\354\0\0\354\0\0\354\0\0\353\0\0\367\230\230\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\367\247\247\351\0\0\352\0\0\352\0\0\352\0\0\352\0\0\352\0\0\352"
		"\0\0\352\0\0\352\0\0\352\0\0\352\0\0\352\0\0\351\0\0\352\0\0\352\0\0\352"
		"\0\0\352\0\0\352\0\0\352\0\0\352\0\0\352\0\0\351\0\0\366\226\226\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\367\243\243\347\0\0\347\0\0\347\0\0\347\0\0"
		"\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0"
		"\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0\347\0\0\365\225\225\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\365\237\237\345"
		"\0\0\344\0\0\344\0\0\344\0\0\344\0\0\344\0\0\344\0\0\344\0\0\344\0\0\344"
		"\0\0\344\0\0\344\0\0\344\0\0\344\0\0\344\0\0\344\0\0\345\0\0\345\0\0\364"
		"\223\223\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\364\234\234\342\0\0\342\0\0\342\0\0\342\0\0\342\0\0"
		"\342\0\0\342\0\0\342\0\0\342\0\0\342\0\0\342\0\0\342\0\0\342\0\0\342\0\0"
		"\342\0\0\343\0\0\363\221\221\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\362\230"
		"\230\341\0\0\340\0\0\340\0\0\340\0\0\340\0\0\340\0\0\340\0\0\340\0\0\340"
		"\0\0\340\0\0\340\0\0\340\0\0\340\0\0\341\0\0\362\220\220\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\361\224\224\336\0\0\336\0\0"
		"\336\0\0\336\0\0\336\0\0\336\0\0\336\0\0\336\0\0\336\0\0\336\0\0\336\0\0"
		"\336\0\0\361\222\222\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\365\262\262\334\0\0\334\0\0\334\0\0\334\0\0\334\0\0"
		"\334\0\0\334\0\0\334\0\0\334\0\0\334\0\0\334\0\0\334\0\0\351ZZ\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\364\260\260\332\0\0\332"
		"\0\0\332\0\0\332\0\0\332\0\0\332\0\0\332\0\0\332\0\0\332\0\0\332\0\0\332"
		"\0\0\332\0\0\332\0\0\332\0\0\350]]\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\363"
		"\260\260\331\0\0\330\0\0\330\0\0\330\0\0\330\0\0\330\0\0\330\0\0\330\0\0"
		"\330\0\0\330\0\0\330\0\0\330\0\0\330\0\0\330\0\0\330\0\0\330\0\0\350bb\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\363\261\261\327\0\0\327\0\0\326\0\0\326\0\0\326\0\0\326\0\0\326"
		"\0\0\326\0\0\326\0\0\326\0\0\326\0\0\326\0\0\326\0\0\326\0\0\326\0\0\326"
		"\0\0\326\0\0\326\0\0\347gg\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\362\262\262\325\0\0\325\0\0\324\0\0\324\0\0\324\0\0\324\0\0"
		"\324\0\0\324\0\0\324\0\0\324\0\0\324\0\0\324\0\0\324\0\0\324\0\0\324\0\0"
		"\324\0\0\324\0\0\324\0\0\324\0\0\324\0\0\347nn\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\362\262\262\323\0\0\323\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322"
		"\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322"
		"\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322\0\0\322\0\0\350ww\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\361"
		"\263\263\321\0\0\321\0\0\320\0\0\320\0\0\320\0\0\320\0\0\320\0\0\320\0\0"
		"\320\0\0\320\0\0\320\0\0\351\203\203\353\217\217\320\0\0\320\0\0\320\0\0"
		"\320\0\0\320\0\0\320\0\0\320\0\0\320\0\0\320\0\0\320\0\0\320\0\0\350\201"
		"\201\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\360\264"
		"\264\317\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316"
		"\0\0\316\0\0\316\0\0\347~~\377\377\377\377\377\377\351\215\215\316\0\0\316"
		"\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316\0\0\316"
		"\0\0\350\212\212\377\377\377\377\377\377\377\377\377\360\265\265\315\0\0"
		"\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0"
		"\314\0\0\345||\377\377\377\377\377\377\377\377\377\377\377\377\351\217\217"
		"\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0\314\0\0"
		"\314\0\0\315\0\0\351\223\223\377\377\377\360\265\265\312\0\0\312\0\0\312"
		"\0\0\312\0\0\312\0\0\312\0\0\312\0\0\312\0\0\312\0\0\312\0\0\312\0\0\344"
		"zz\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\351\221\221\312\0\0\312\0\0\312\0\0\312\0\0\312\0\0\312\0\0\312\0\0\312"
		"\0\0\312\0\0\312\0\0\313\0\0\352\231\231\331OO\311\0\0\311\0\0\311\0\0\311"
		"\0\0\311\0\0\311\0\0\311\0\0\311\0\0\311\0\0\311\0\0\342xx\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\350\223\223\311\0\0\311\0\0\311\0\0\311\0\0\311\0\0\311\0\0\311"
		"\0\0\311\0\0\311\0\0\311\0\0\337gg\377\377\377\331QQ\307\0\0\310\0\0\310"
		"\0\0\310\0\0\310\0\0\310\0\0\310\0\0\307\0\0\341vv\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\350\225\225\307\0\0\310\0\0\310\0\0\310\0\0\310"
		"\0\0\310\0\0\310\0\0\307\0\0\340rr\377\377\377\377\377\377\377\377\377\332"
		"]]\305\0\0\305\0\0\306\0\0\306\0\0\306\0\0\305\0\0\337tt\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\347\227\227\305"
		"\0\0\306\0\0\306\0\0\306\0\0\306\0\0\305\0\0\343\204\204\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\333ff\303\0\0\304\0\0\304\0"
		"\0\303\0\0\336rr\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\347\230\230\303\0\0\304\0\0\304"
		"\0\0\303\0\0\346\223\223\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\336tt\301\0\0\301\0\0\334pp\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\346\231\231\301\0\0\301\0\0\350"
		"\236\236\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\341\205\205\333oo\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
		"\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\345\226\226"
		"\350\245\245\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377",
	};

}