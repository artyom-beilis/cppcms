#ifndef CPPCMS_ENCODING_VALIDATORS_H
#define CPPCMS_ENCODING_VALIDATORA_H

namespace cppcms { namespace encoding {

	// Based on: http://www.w3.org/International/questions/qa-forms-utf-8

	template<typename Iterator>
	bool utf8_valid(Iterator p,Iterator e)
	{
		for(;p!=e;++p) {
			unsigned char c=*p;
			unsigned char seq0,seq1=0,seq2=0,seq3=0;
			seq0=c;
			int len=1;
			if((c & 0xC0) == 0xC0) {
				++p;
				if(p==e)
					return false;
				seq1=*p;
				len=2;
			}
			if((c & 0xE0) == 0xE0) {
				++p;
				if(p==e)
					return false;
				seq2=*p;
				len=3;
			}
			if((c & 0xF0) == 0xF0) {
				++p;
				if(p==e)
					return false;
				seq3=*p;
				len=4;
			}
			switch(len) {
			case 1: // ASCII
				if(seq0==0x9 || seq0==0x0A || seq0==0x0D || (0x20<=seq0 && seq0<=0x7E))
					break;
				return false;
			case 2: // non-overloading 2 bytes
				if(0xC2 <= seq0 && seq0 <= 0xDF) {
					if(0x80 <= seq1 && seq1<= 0xBF)
						break;
				}
				return false;
			case 3: 
				if(seq0==0xE0) { // exclude overloadings
					if(0xA0 <=seq1 && seq1<= 0xBF && 0x80 <=seq2 && seq2<=0xBF)
						break;
				}
				else if( (0xE1 <= seq0 && seq0 <=0xEC) || seq0==0xEE || seq0==0xEF) { // stright 3 bytes
					if(	0x80 <=seq1 && seq1<=0xBF &&
						0x80 <=seq2 && seq2<=0xBF)
						break;
				}
				else if(seq0 == 0xED) { // exclude surrogates
					if(	0x80 <=seq1 && seq1<=0x9F &&
						0x80 <=seq2 && seq2<=0xBF)
						break;
				}
				return false;
			case 4:
				switch(seq0) {
				case 0xF0: // planes 1-3
					if(	0x90 <=seq1 && seq1<=0xBF &&
						0x80 <=seq2 && seq2<=0xBF &&
						0x80 <=seq3 && seq3<=0xBF)
						break;
					return false;
				case 0xF1: // planes 4-15
				case 0xF2:
				case 0xF3:
					if(	0x80 <=seq1 && seq1<=0xBF &&
						0x80 <=seq2 && seq2<=0xBF &&
						0x80 <=seq3 && seq3<=0xBF)
						break;
					return false;
				case 0xF4: // pane 16
					if(	0x80 <=seq1 && seq1<=0x8F &&
						0x80 <=seq2 && seq2<=0xBF &&
						0x80 <=seq3 && seq3<=0xBF)
						break;
					return false;
				default:
					return false;
				}


			}
		}
		return true;
	} // valid
	

	template<typename Iterator>
	bool ascii_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1E< c)
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool iso_8859_1_2_4_5_9_10_13_14_15_16_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool iso_8859_3_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xA5:
			case 0xAE:
			case 0xBE:
			case 0xC3:
			case 0xD0:
			case 0xE3:
			case 0xF0:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool iso_8859_6_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			if(	(0xA1<=c && c<=0xA3) ||
			 	(0xA5 <=c && c<= 0xAB) || 
				(0xAE <=c && c<= 0xBA) ||
				(0xBC <=c && c<= 0xBE) ||
				 0xC0 == c		||
				(0xDB <=c && c<= 0xDF) ||
				(0xF3 <=c && c<= 0xFF))
			{
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool iso_8859_7_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xAE:
			case 0xD2:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool iso_8859_8_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xA1:
			case 0xFB:
			case 0xFC:
			case 0xFF:
				return false;
			}
			if(0xBF <=c && c<=0xDE)
				return false;
		}
		return true;
	}

	template<typename Iterator>
	bool iso_8859_11_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xDB:
			case 0xDC:
			case 0xDD:
			case 0xDE:
			case 0xFC:
			case 0xFD:
			case 0xFE:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1250_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x83:
			case 0x88:
			case 0x90:
			case 0x98:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1251_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c || c==152)
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1252_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x8D:
			case 0x8F:
			case 0x90:
			case 0x9D:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1253_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x88:
			case 0x8A:
			case 0x8C:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			case 0x90:
			case 0x98:
			case 0x9A:
			case 0x9C:
			case 0x9D:
			case 0x9E:
			case 0x9F:
			case 0xAA:
			case 0xD2:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1254_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			case 0x90:
			case 0x9D:
			case 0x9E:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1255_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x8A:
			case 0x8C:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			
			case 0x90:
			case 0x9A:
			case 0x9C:
			case 0x9D:
			case 0x9E:
			case 0x9F:

			case 0xD9:
			case 0xDA:
			case 0xDB:
			case 0xDC:
			case 0xDD:
			case 0xDE:
			case 0xDF:

			case 0xCA:
			case 0xFB:
			case 0xFC:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1256_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1257_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x83:
			case 0x88:
			case 0x8A:
			case 0x8C:
			case 0x90:
			case 0x98:
			case 0x9A:
			case 0x9C:
			case 0x9F:
			case 0xA1:
			case 0xA5:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1258_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x8A:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			case 0x90:
			case 0x9A:
			case 0x9D:
			case 0x9E:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool koi8_valid(Iterator p,Iterator e)
	{
		while(p!=e) {
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x1F==c)
				return false;
		}
		return true;
	}

	// ISO 


} /* validator */ } // cppcms

#endif
