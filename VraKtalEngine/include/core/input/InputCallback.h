struct InputCallback
{
	void* context = nullptr; // Objet si data
	void (*callback)(void*) = nullptr; // la fonction

	void Execute() const
	{
		if (callback)
			callback(context);
	}
};