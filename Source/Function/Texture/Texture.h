#pragma once
#include "Core/Math/Math.h"
#include "stb/stb_image.h"

class Sampler;

struct Image {
	uint32_t width{ 0 }, height{ 0 }, BPP{ 32 };
	uint64_t imageSize{ 0 };
	stbi_uc* source{ nullptr };
};

class Texture {
public:
	Texture() { images = new Image[1]; }
	Texture(size_t subresourceCount) : subresourceCount(subresourceCount) { images = new Image[subresourceCount]; }
	Texture(const char* path, uint32_t BPP) { LoadImageWithSTB(path, BPP); }
	void Release();

	void LoadImageWithSTB(const char* path, uint32_t BPP, size_t subresource);
	void LoadImageWithSTB(const char* path, uint32_t BPP);

	Vector4f Sample(Sampler sampler, Vector2f coord)const;

private:
	size_t subresourceCount{ 1 };
	Image* images{ nullptr };
};

class Sampler {
public:
	Sampler() {}

	enum class Filter {
		Nearest = 0,
		Linear
	};
	enum class AddressMode {
		Repeat = 0,
		Mirror,
		Clamp,
		Border
	};

	friend class Texture;

	Sampler(Filter filter, AddressMode addressModeU, AddressMode addressModeV)
	: filter(filter), addressModeU(addressModeU), addressModeV(addressModeV) {}

	void SetBorderColor(Vector4f color) { borderColor = color; }

private:
	float Address(AddressMode addressMode, float coord)const;

	Filter filter{ Filter::Nearest };
	AddressMode addressModeU{ AddressMode::Repeat };
	AddressMode addressModeV{ AddressMode::Repeat };
	Vector4f borderColor{ Vector4f(0.0f, 0.0f, 0.0f, 0.0f) };
};