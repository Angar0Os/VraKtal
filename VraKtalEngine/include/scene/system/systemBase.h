#pragma once
class Scene;

class SystemBase
{
public:
	SystemBase() = default;
	virtual ~SystemBase() = default;

	virtual void Update(Scene& world) = 0;

	void SetID(unsigned char id) { ID = id; }
	unsigned char GetID() const { return ID; }
private:
	unsigned char ID; // Unique identifier 
};