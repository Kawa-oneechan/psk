#include <ctime>
#include <numeric>
#include <glad/glad.h>
#include <stb_image_write.h>
#include "Utilities.h"
#include "Console.h"
#include "InputsMap.h"
#include "Tickable.h"
#include "JsonUtils.h"

extern int width, height;

#pragma region Point in Shape detection
bool PointInPoly(const glm::vec2 point, const polygon& polygon)
{
	int crossings = 0;
	const auto numPts = polygon.size() - 1;

	for (auto i = 0; i < numPts; i++)
	{
		if (((polygon[i].y <= point.y) && (polygon[i + 1].y > point.y))
			|| ((polygon[i].y > point.y) && (polygon[i + 1].y <= point.y)))
		{
			auto vt = (point.y - polygon[i].y) / (polygon[i + 1].y - polygon[i].y);
			if (point.x < polygon[i].x + vt * (polygon[i + 1].x - polygon[i].x))
			{
				++crossings;
			}
		}
	}
	return (crossings & 1) == 1;
}

bool PointInRect(const glm::vec2 point, const glm::vec4 rect)
{
	return
		(point.x >= rect.x) &&
		(point.x < rect.z) &&
		(point.y >= rect.y) &&
		(point.y < rect.w);
}
#pragma endregion

#pragma region Color spaces
//From https://bottosson.github.io/posts/oklab/
glm::vec3 LinearSRGBtoOkLab(const glm::vec3& c)
{
	float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
	float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
	float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

	float l_ = cbrtf(l);
	float m_ = cbrtf(m);
	float s_ = cbrtf(s);

	return glm::vec3(
		0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
		1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
		0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_
		);
}

glm::vec3 OkLabToLinearSRGB(const glm::vec3& c)
{
	float l_ = c.r + 0.3963377774f * c.g + 0.2158037573f * c.b;
	float m_ = c.r - 0.1055613458f * c.g - 0.0638541728f * c.b;
	float s_ = c.r - 0.0894841775f * c.g - 1.2914855480f * c.b;

	float l = l_ * l_ * l_;
	float m = m_ * m_ * m_;
	float s = s_ * s_ * s_;

	return glm::vec3(
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s
		);
}

//From https://gist.github.com/flowb/6325155c7cb5f8e07cc9837e845fe780
glm::vec3 RGBtoHSV(const glm::vec3& c)
{
	auto max = glm::max(c.r, glm::max(c.g, c.b)); //Value
	auto min = glm::min(c.r, glm::min(c.g, c.b));

	auto delta = max - min;

	auto sat = delta / (max + glm::step(max, 0.0f)); //Saturation

	/*
	This is a vec3 wrapped around 3 boolean evaluations
	cast to floats. it's used to figure out which third
	of the hue cycle we are in. Only one of the three 
	components should be true and be cast to 1.0.
	*/
	auto isMax = glm::vec3(
		float(c.r >= c.g && c.r >= c.b), //Yellow to magenta
		float(c.g > c.r && c.g >= c.b),  //Yellow to cyan
		float(c.b > c.r && c.b > c.g)	 //Magenta to cyan
	);

	/*
	For each channel: get the difference between the other
	two channels. then divide all by the overall delta.
	*/
	auto secondary = glm::vec3(
		(c.g - c.b),
		(c.b - c.r),
		(c.r - c.g))
		/ (delta + glm::step(delta, 0.0f));

	/*
	Pad the G and B channels to make a 1/3 phase offset in
	a [0-5] domain.
	*/
	secondary += glm::vec3(0.0f, 2.0f, 4.0f);

	/*
	The ternary catches achromatic input colors and 
	returns hue = 0.0 instead. this could be probably be 
	done with funky step/mix voodoo instead but IMHO the 
	branch here is a small price to pay for readibility.
	the dot product performs componentwise multiplication 
	and sums all components. this lets us take all three
	secondary deltas, scales the ones we don't need to
	zero and retain the result we actually need.
	*/
	auto hue = sat > 0.0f ?
		glm::fract(dot(isMax, secondary) / 6.0f) :
		0.0f;

	return glm::vec3(hue, sat, max);
}

glm::vec3 HSVtoRGB(const glm::vec3& hsv)
{
	/*E
	valuates three phase-shifted triangle wave functions
	for the input of hue to get fully saturated R, G and B.
	*/
	auto rgb = glm::clamp(
		glm::abs(
			glm::mod(
				hsv.x * 6.0f + glm::vec3(0.0f, 4.0f, 2.0f),
				6.0f) -
			3.0f) -
		1.0f, 0.0f, 1.0f);

	/*
	Mixes between white and fully saturated RGB
	then scales by value
	*/
	return glm::mix(glm::vec3(1.0f), rgb, hsv.y) * hsv.z;
}

//From various places
glm::vec3 RGBtoHSL(const glm::vec3& c)
{
	auto hsv = RGBtoHSV(c);
	auto l = hsv.z * (1.0f - (hsv.y * 0.5f));
	return glm::vec3(hsv.x, (l == 0.0f || l == 1.0f) ? 0 : (hsv.z - l) / glm::min(l, 1.0f - l), l);
}

glm::vec3 HSLtoRGB(const glm::vec3& hsl)
{
	auto rgb = glm::clamp(
		glm::abs(
			glm::mod(
				hsl.x * 6.0f + glm::vec3(0.0f, 4.0f, 2.0f),
				6.0f) -
			3.0f) -
		1.0f, 0.0f, 1.0f);
	auto c = (1.0f - glm::abs(2.0f * hsl.z - 1.0f)) * hsl.y;
	return (rgb - 0.5f) * c + hsl.z;
}
#pragma endregion

void Screenshot()
{
	auto pixels = new unsigned char[3 * width * height];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	stbi_flip_vertically_on_write(1);
	char filename[128];
	auto now = time(NULL);
	tm gm {};
	localtime_s(&gm, &now);
	std::strftime(filename, 128, "%Y%m%d_%H%M%S.png", &gm);
	stbi_write_png(filename, width, height, 3, pixels, width * 3);
	delete[] pixels;
	conprint(0, "Screenshot taken: {}", filename);
}

#pragma region CRC32
static constexpr unsigned int crcLut[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

hash GetCRC(const std::string& text)
{
	//unsigned int crc = 0xFFFFFFFFL;
	//for (auto c : text)
	//	crc = (crc >> 8) ^ crcLut[c ^ crc & 0xFF];
	//return crc ^ 0xFFFFFFFFL;
	return std::accumulate(text.begin(), text.end(), 0xFFFFFFFFL, [](auto crc, auto chr)
	{
		return (crc >> 8) ^ crcLut[chr ^ crc & 0xFF];
	}) ^ 0xFFFFFFFFL;
}

hash GetCRC(unsigned char *buffer, int len) // cppcheck-suppress constParameterPointer
{
	unsigned int crc = 0xFFFFFFFFL;

	for (auto i = 0; i < len; i++)
		crc = (crc >> 8) ^ crcLut[buffer[i] ^ crc & 0xFF];

	return crc ^ 0xFFFFFFFFL;
}

static constexpr unsigned int constexo_crc32_helper(const char* text, size_t size,
	unsigned int crc)
{
	return size == 0 ?
		~crc :
		constexo_crc32_helper(&text[1], size - 1,
			crcLut[((crc) ^ (text[0])) & 0xff] ^ ((crc) >> 8));
}

constexpr hash operator ""_crc(const char* text, size_t size)
{
	return constexo_crc32_helper(text, size, 0xFFFFFFFF);
}

extern "C"
{
	unsigned long mz_crc32(unsigned long start, const unsigned char *ptr, size_t buf_len)
	{
		unsigned int crc = start ^ 0xFFFFFFFFL;

		for (auto i = 0; i < buf_len; i++)
			crc = (crc >> 8) ^ crcLut[ptr[i] ^ crc & 0xFF];

		return crc ^ 0xFFFFFFFFL;
	}
}
#pragma endregion
