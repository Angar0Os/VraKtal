enum EInterpolationType
{
	
};

template<typename T>
struct Keyframe
{
	float time = 0;
	EInterpolationType interpolation;
	T property;
};