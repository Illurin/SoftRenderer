#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"

void Texture::Release() {
	for (size_t i = 0; i < subresourceCount; i++) {
		stbi_image_free(images[i].source);
	}
	delete[] images;
}

void Texture::LoadImageWithSTB(const char* path, uint32_t BPP, size_t subresource) {
	auto& image = images[subresource];
	image.BPP = BPP;
	int channelInFile;
	stbi_uc* source = stbi_load(path, reinterpret_cast<int*>(&image.width), reinterpret_cast<int*>(&image.height), &channelInFile, 4);

	uint64_t pixelRowPitch = ((uint64_t)image.width * (uint64_t)BPP + 7) / 8;
	image.imageSize = pixelRowPitch * (uint64_t)image.height;

	image.source = reinterpret_cast<uint8_t*>(source);
}

void Texture::LoadImageWithSTB(const char* path, uint32_t BPP) {
	LoadImageWithSTB(path, BPP, 0);
}

Vector4f Texture::Sample(Sampler sampler, Vector2f coord)const {
	float x = sampler.Address(sampler.addressModeU, coord.x);
	float y = sampler.Address(sampler.addressModeV, coord.y);
	if (x == -1.0f || y == -1.0f)
		return sampler.borderColor;
	auto& image = images[0];

	y = 1.0f - y;

	x = x * (float)image.width;
	y = y * (float)image.height;

	x -= 0.5f;
	y -= 0.5f;

	int width = x;
	int height = y;

	if (x - width > 0.5f)width++;
	if (x - height > 0.5f)height++;

	int byte = image.BPP / 32;

	switch (sampler.filter) {
	case Sampler::Filter::Nearest: {
		Vector4f color;
		color.x = (float)image.source[height * image.width * byte * 4 + width * 4] / 255.0f;
		color.y = (float)image.source[height * image.width * byte * 4 + width * 4 + 1] / 255.0f;
		color.z = (float)image.source[height * image.width * byte * 4 + width * 4 + 2] / 255.0f;
		color.w = (float)image.source[height * image.width * byte * 4 + width * 4 + 3] / 255.0f;
		return color;
	}
	case Sampler::Filter::Linear: {
		Vector4f color[4];
		for (int i = 0; i < 4; i++) {
			if (i == 1 && x > 0.0f && x < 9.0f)
				width++;
			if (i == 2 && x > 0.0f && x < 9.0f)
				width--;
			if (i == 3 && x > 0.0f && x < 9.0f)
				width++;
			if (i == 2 && y > 0.0f && y < 9.0f)
				height++;

			color[i].x = (float)image.source[height * image.width * byte * 4 + width * 4] / 255.0f;
			color[i].y = (float)image.source[height * image.width * byte * 4 + width * 4 + 1] / 255.0f;
			color[i].z = (float)image.source[height * image.width * byte * 4 + width * 4 + 2] / 255.0f;
			color[i].w = (float)image.source[height * image.width * byte * 4 + width * 4 + 3] / 255.0f;
		}

		float lerpX = (x - width) / 1.0f;
		float lerpY = (y - height) / 1.0f;

		color[0] = Lerp(color[0], color[1], lerpX);
		color[2] = Lerp(color[2], color[3], lerpX);
		return Lerp(color[0], color[2], lerpY);
	}
	}
}

float Sampler::Address(AddressMode addressMode, float coord)const {
	switch (addressMode) {
	case AddressMode::Repeat:
		while (coord > 1.0f) {
			coord -= 1.0f;
		}
		while (coord < 0.0f) {
			coord += 1.0f;
		}
		break;
	case AddressMode::Mirror:
	{
		bool mirror = false;
		while (coord > 1.0f) {
			coord -= 1.0f;
			mirror = !mirror;
		}
		while (coord < 0.0f) {
			coord += 1.0f;
			mirror = !mirror;
		}
		if (mirror)
			coord = 1.0f - coord;
		break;
	}
	case AddressMode::Clamp:
		coord = coord > 1.0f ? 1.0f : coord;
		coord = coord < 0.0f ? 0.0f : coord;
		break;
	case AddressMode::Border:
		coord = coord > 1.0f || coord < 0.0f ? -1.0f : coord;
		break;
	}
	return coord;
}