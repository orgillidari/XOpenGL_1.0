#include "XFront.h"

namespace Smile
{
	XFront::XFront() 
	{

	}

	XFront::~XFront() 
	{

	}

	void XFront::Init(const char* pFront, int texW, int texH, int size)
	{
		/****************************************************************************************************************
		 *
		 *    Brief   : ��ʼ��Freetype�����á�
		 *
		 ****************************************************************************************************************/
		FT_Error error = NULL;
		error = FT_Init_FreeType(&_Library);
		error = FT_New_Face(_Library, pFront, 0, &_Face);
		error = FT_Select_Charmap(_Face, FT_ENCODING_UNICODE);

		_Size = size;
		FT_F26Dot6 ftSize = (FT_F26Dot6)(_Size * (1 << 6));
		FT_Set_Char_Size(_Face, ftSize, 0, 72, 72);

		/****************************************************************************************************************
		 *
		 *    Brief   : ���� - �ַ������붥�㻺�档
		 *
		 ****************************************************************************************************************/
		_TextureW = texW;
		_TextureH = texH;

		glGenTextures(1, &_Texture);
		glBindTexture(GL_TEXTURE_2D, _Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, _TextureW, _TextureH, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);

		_Characters = new Character[1 << 16];
		memset(_Characters, 0, sizeof(_Characters));

		_FrontVertices = new FrontVertex[1 << 16];
		memset(_FrontVertices, 0, sizeof(_FrontVertices));

		/****************************************************************************************************************
		 *
		 *    Brief   : ����
		 *
		 ****************************************************************************************************************/
		_offsetX = 0.0f;
		_offsetY = 0.0f;

		_FrontMaxX = 0.0f;
		_FrontMaxY = 0.0f;
	}

	void XFront::Done()
	{
		FT_Error error = 0;
		error = FT_Done_Face(_Face);
		error = FT_Done_FreeType(_Library);

		glDeleteTextures(1, &_Texture);

		delete[] _Characters;
		delete[] _FrontVertices;
	}

	void XFront::Begin(int screenW, int screenH)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, screenW, 0, screenH, -100, 100);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _Texture);
	}

	void XFront::End()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	XRectf XFront::Draw(float x, float y, float z, BGRA8U color,const wchar_t* text)
	{
		XRectf rect = XRectf(0.0f, 0.0f, 0.0f, 0.0f);

		unsigned int len = wcslen(text);
		if (len == 0)
			return rect;

		/****************************************************************************************************************
		 *
		 *    Brief   : ���ɶ�������
		 *
		 ****************************************************************************************************************/		
		float startX = x;
		float startY = y;
		float startZ = z;

		float maxY = y;
		float minY = y;

		float maxOffset = 0.0f;

		FrontVertex* pVertices = _FrontVertices;

		for (unsigned int i = 0; i < len; ++i)
		{
			wchar_t c = text[i];
			Character ch = GetCharacter(c);

			float offsetX = ch._Offset._x;
			float offsetY = -(ch._Size._y - ch._Offset._y);

			if (i == 0)
			{
				rect._x = startX + offsetX;
			}

			//��һ��������
			pVertices[i * 6 + 0]._Pos._x = startX + offsetX;
			pVertices[i * 6 + 0]._Pos._y = startY + offsetY;
			pVertices[i * 6 + 0]._Pos._z = startZ;
			pVertices[i * 6 + 0]._UV._u = ch._Pos._x / _TextureW;
			pVertices[i * 6 + 0]._UV._v = (ch._Pos._y + ch._Size._y) / _TextureH;
			pVertices[i * 6 + 0]._Color = color;

			pVertices[i * 6 + 1]._Pos._x = startX + offsetX + ch._Size._x;
			pVertices[i * 6 + 1]._Pos._y = startY + offsetY;
			pVertices[i * 6 + 1]._Pos._z = startZ;
			pVertices[i * 6 + 1]._UV._u = (ch._Pos._x + ch._Size._x) / _TextureW;
			pVertices[i * 6 + 1]._UV._v = (ch._Pos._y + ch._Size._y) / _TextureH;
			pVertices[i * 6 + 1]._Color = color;

			pVertices[i * 6 + 2]._Pos._x = startX + offsetX + ch._Size._x;
			pVertices[i * 6 + 2]._Pos._y = startY + offsetY + ch._Size._y;
			pVertices[i * 6 + 2]._Pos._z = startZ;
			pVertices[i * 6 + 2]._UV._u = (ch._Pos._x + ch._Size._x) / _TextureW;
			pVertices[i * 6 + 2]._UV._v = ch._Pos._y / _TextureH;
			pVertices[i * 6 + 2]._Color = color;

			//�ڶ���������
			pVertices[i * 6 + 3]._Pos._x = startX + offsetX;
			pVertices[i * 6 + 3]._Pos._y = startY + offsetY;
			pVertices[i * 6 + 3]._Pos._z = startZ;
			pVertices[i * 6 + 3]._UV._u = ch._Pos._x / _TextureW;
			pVertices[i * 6 + 3]._UV._v = (ch._Pos._y + ch._Size._y) / _TextureH;
			pVertices[i * 6 + 3]._Color = color;

			pVertices[i * 6 + 4]._Pos._x = startX + offsetX + ch._Size._x;
			pVertices[i * 6 + 4]._Pos._y = startY + offsetY + ch._Size._y;
			pVertices[i * 6 + 4]._Pos._z = startZ;
			pVertices[i * 6 + 4]._UV._u = (ch._Pos._x + ch._Size._x) / _TextureW;
			pVertices[i * 6 + 4]._UV._v = ch._Pos._y / _TextureH;
			pVertices[i * 6 + 4]._Color = color;

			pVertices[i * 6 + 5]._Pos._x = startX + offsetX;
			pVertices[i * 6 + 5]._Pos._y = startY + offsetY + ch._Size._y;
			pVertices[i * 6 + 5]._Pos._z = startZ;
			pVertices[i * 6 + 5]._UV._u = ch._Pos._x / _TextureW;
			pVertices[i * 6 + 5]._UV._v = ch._Pos._y / _TextureH;
			pVertices[i * 6 + 5]._Color = color;

			//ʹ�������advance��Ϊ��ȡ�
			startX += ch._Span._x;

			//�������ֱ߿��С��
			maxY = max(maxY, pVertices[i * 6 + 2]._Pos._y);
			minY = min(minY, pVertices[i * 6 + 0]._Pos._y);

			maxOffset = min(maxOffset, offsetY);

			rect._w += ch._Span._x;
			rect._h = max(rect._y, maxY - minY);
		}

		rect._y = startY + maxOffset;


		/****************************************************************************************************************
		 *
		 *    Brief   : ����
		 *
		 ****************************************************************************************************************/
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		
		glVertexPointer(3, GL_FLOAT, sizeof(FrontVertex), &pVertices[0]._Pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(FrontVertex), &pVertices[0]._UV);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(FrontVertex), &pVertices[0]._Color);

		glDrawArrays(GL_TRIANGLES, 0, len * 6);

		FrontVertex poses[4] = {};

		poses[0]._Pos = XVec3f(rect._x, rect._y, 0.0f);
		poses[0]._UV = XVec2f(0, 0);
		poses[0]._Color = BGRA8U(255, 0, 0, 255);

		poses[1]._Pos = XVec3f(rect._x + rect._w, rect._y, 0.0f);
		poses[1]._UV = XVec2f(0, 0);
		poses[1]._Color = BGRA8U(255, 0, 0, 255);

		poses[2]._Pos = XVec3f(rect._x + rect._w, rect._y + rect._h, 0.0f);
		poses[2]._UV = XVec2f(0, 0);
		poses[2]._Color = BGRA8U(255, 0, 0, 255);

		poses[3]._Pos = XVec3f(rect._x, rect._y + rect._h, 0.0f);
		poses[3]._UV = XVec2f(0, 0);
		poses[3]._Color = BGRA8U(255, 0, 0, 255);

		glVertexPointer(3, GL_FLOAT, sizeof(FrontVertex), &poses[0]._Pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(FrontVertex), &poses[0]._UV);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(FrontVertex), &poses[0]._Color);

		glDrawArrays(GL_LINE_LOOP, 0, 4);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		return rect;
	}

	XRectf XFront::GetSize(float x, float y, float z, const wchar_t* text)
	{
		XRectf rect = XRectf(0.0f, 0.0f, 0.0f, 0.0f);

		unsigned int len = wcslen(text);
		if (len == 0)
			return rect;

		/****************************************************************************************************************
		*
		*    Brief   : ���ɶ�������
		*
		****************************************************************************************************************/
		float startX = x;
		float startY = y;
		float startZ = z;

		float maxY = y;
		float minY = y;

		float maxOffset = 0.0f;

		FrontVertex* pVertices = _FrontVertices;

		for (unsigned int i = 0; i < len; ++i)
		{
			wchar_t c = text[i];
			Character ch = GetCharacter(c);

			float offsetX = ch._Offset._x;
			float offsetY = -(ch._Size._y - ch._Offset._y);

			if (i == 0)
			{
				rect._x = startX + offsetX;
			}

			//��һ��������
			pVertices[i * 6 + 0]._Pos._x = startX + offsetX;
			pVertices[i * 6 + 0]._Pos._y = startY + offsetY;
			pVertices[i * 6 + 0]._Pos._z = startZ;

			pVertices[i * 6 + 2]._Pos._x = startX + offsetX + ch._Size._x;
			pVertices[i * 6 + 2]._Pos._y = startY + offsetY + ch._Size._y;
			pVertices[i * 6 + 2]._Pos._z = startZ;

			//ʹ�������advance��Ϊ��ȡ�
			startX += ch._Span._x;

			//�������ֱ߿��С��
			maxY = max(maxY, pVertices[i * 6 + 2]._Pos._y);
			minY = min(minY, pVertices[i * 6 + 0]._Pos._y);

			maxOffset = min(maxOffset, offsetY);

			rect._w += ch._Span._x;
			rect._h = max(rect._y, maxY - minY);
		}

		rect._y = startY + maxOffset;

		return rect;
	}

	Character& XFront::GetCharacter(unsigned int cIndex)
	{
		if (_Characters[cIndex]._found)
			return _Characters[cIndex];
			
		//��ȡ��������
		FT_UInt index = FT_Get_Char_Index(_Face, cIndex);

		//��������
		FT_Error error = FT_Load_Glyph(_Face, index, FT_LOAD_DEFAULT);

		//��ȡ����
		FT_Glyph glyph;
		FT_Get_Glyph(_Face->glyph, &glyph);

		//ת����λͼ��ʹ�ÿ����
		FT_Glyph_To_Bitmap(&glyph, ft_render_mode_mono, 0, 1);
		FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
		FT_Bitmap& bitmap = bitmapGlyph->bitmap;

		//����ת��������С���岻��������ʾ��
		FT_Bitmap newBitmap;
		FT_Bitmap_New(&newBitmap);
		FT_Bitmap* pBitmap = &bitmap;
		if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
		{
			if (FT_Bitmap_Convert(_Library, &bitmap, &newBitmap, 1) == 0)
			{
				/**
				*   Go through the bitmap and convert all of the nonzero values to 0xFF (white).
				*/
				for (unsigned char* p = newBitmap.buffer, *endP = p + newBitmap.width * newBitmap.rows; p != endP; ++p)
					*p ^= -*p ^ *p;
				pBitmap = &newBitmap;
			}
		}

		//�������ݡ�
		if (pBitmap->width == 0 || pBitmap->rows == 0)
		{
			//�������ֵ
			_FrontMaxX = max(_FrontMaxX, _Size / 2.0f);
			_FrontMaxY = max(_FrontMaxY, _Size);

			//������
			if (_offsetX + _Size / 2.0f >= _TextureW)
			{
				//д��һ��,���¿�ʼ
				_offsetX = 0;
				_offsetY += _FrontMaxY;

				//�����������С���Ҫ�����µ�������������������ʹ�õ������ܴ��������ڲ���Ҫ��
				// ......
			}

			char mem[1024] = {};

			//������޷���ʾ������ʱ������ո���ʹ�ð���ַ��ռ䡣
			_Characters[cIndex]._found = true;

			_Characters[cIndex]._Texture = _Texture;

			_Characters[cIndex]._Pos._x = _offsetX;
			_Characters[cIndex]._Pos._y = _offsetY;

			_Characters[cIndex]._Size._x = _Size / 2.0f;
			_Characters[cIndex]._Size._y = 0;

			_Characters[cIndex]._Offset._x = bitmapGlyph->left;
			_Characters[cIndex]._Offset._y = bitmapGlyph->top;

			_Characters[cIndex]._Span._x = _Face->glyph->advance.x / 64;
			_Characters[cIndex]._Span._y = _Face->glyph->advance.y / 64;

			glBindTexture(GL_TEXTURE_2D, _Texture);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexSubImage2D(GL_TEXTURE_2D, 0, _offsetX, _offsetY, _Size / 2.0f, _Size, GL_ALPHA, GL_UNSIGNED_BYTE, mem);
			glBindTexture(GL_TEXTURE_2D, 0);

			_offsetX += _Size / 2.0f + 1;
		}
		else
		{
			//�������ֵ
			_FrontMaxX = max(_FrontMaxX, pBitmap->width);
			_FrontMaxY = max(_FrontMaxY, pBitmap->rows);

			//������
			if (_offsetX + pBitmap->width >= _TextureW)
			{
				//д��һ��,���¿�ʼ
				_offsetX = 0;
				_offsetY += _FrontMaxY;

				//�����������С���Ҫ�����µ�������������������ʹ�õ������ܴ��������ڲ���Ҫ��
				// ......
			}

			_Characters[cIndex]._found = true;

			_Characters[cIndex]._Texture = _Texture;

			_Characters[cIndex]._Pos._x = _offsetX;
			_Characters[cIndex]._Pos._y = _offsetY;

			_Characters[cIndex]._Size._x = pBitmap->width;
			_Characters[cIndex]._Size._y = pBitmap->rows;

			_Characters[cIndex]._Offset._x = bitmapGlyph->left;
			_Characters[cIndex]._Offset._y = bitmapGlyph->top;

			_Characters[cIndex]._Span._x = _Face->glyph->advance.x / 64;
			_Characters[cIndex]._Span._y = _Face->glyph->advance.y / 64;

			glBindTexture(GL_TEXTURE_2D, _Texture);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexSubImage2D(GL_TEXTURE_2D, 0, _offsetX, _offsetY, pBitmap->width, pBitmap->rows, GL_ALPHA, GL_UNSIGNED_BYTE, pBitmap->buffer);

			//������
			_offsetX += pBitmap->width + 1;
		}

		//����
		FT_Bitmap_Done(_Library, &newBitmap);

		//����
		return _Characters[cIndex];
	}
}