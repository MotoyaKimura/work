#pragma once
#include "Model.h"

class Wrapper;
class Camera;
class AssimpModel : public Model
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Camera> _camera;
	std::string _filePath;

	bool Load() override;
	void ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh);
	void ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial);
	bool ModelHeapInit() override;
	void Draw() override;
	void Update(bool isStart) override;
public:
	AssimpModel(std::shared_ptr<Wrapper> dx, 
		std::shared_ptr<Camera> camera,
		std::string filePath
	);
	~AssimpModel() override;
};
