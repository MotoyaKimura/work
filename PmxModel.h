#pragma once
#include <future>
#include "Model.h"
#include "VMD.h"

enum class PMXVertexWeight : unsigned char
{
	BDEF1,
	BDEF2,
	BDEF4,
	QDEF,//もしかしたらSDEFと逆かも知れない
	SDEF,
};

enum class PMXDrawModeFlags : unsigned char
{
	DoubleSided = 0x01,
	GroundShadow = 0x02,
	SelfShadowMap = 0x04,
	SelfShadow = 0x08,
	Edge = 0x10,
};

enum class PMXSphereMode : unsigned char
{
	None,
	Mul,
	Add,
	SubTexture,
};

enum class PMXToonMode : unsigned char
{
	Separate,
	Common,
};

enum class PMXBoneFlags : uint16_t
{
	TargetShowMode = 0x0001,
	AllowRotate = 0x0002,
	AllowTranslate = 0x0004,
	Visible = 0x0008,
	AllowControl = 0x0010,
	IK = 0x0020,
	AppendLocal = 0x0080,
	AppendRotate = 0x0100,
	AppendTranslate = 0x0200,
	FixedAxis = 0x0400,
	LocalAxis = 0x0800,
	DeformAfterPhysics = 0x1000,
	DeformOuterParent = 0x2000,
};

enum class PMXMorphType : uint8_t
{
	Group,
	Position,
	Bone,
	UV,
	AddUV1,
	AddUV2,
	AddUV3,
	AddUV4,
	Material,
	Flip,
	Impulse,
};

struct PMXMorph
{
	std::wstring name;
	std::string englishName;

	unsigned char controlPanel;
	PMXMorphType morphType;

	struct PositionMorph
	{
		unsigned int vertexIndex;
		DirectX::XMFLOAT3 position;
	};

	struct UVMorph
	{
		unsigned int vertexIndex;
		DirectX::XMFLOAT4 uv;
	};

	struct BoneMorph
	{
		unsigned int boneIndex;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 quaternion;
	};

	struct  MaterialMorph
	{
		enum class OpType : uint8_t
		{
			Mul,
			Add,
		};

		unsigned int materialIndex;
		OpType opType;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT3 specular;
		float specularPower;
		DirectX::XMFLOAT3 ambient;
		DirectX::XMFLOAT4 edgeColor;
		float edgeSize;
		DirectX::XMFLOAT4 textureFactor;
		DirectX::XMFLOAT4 sphereTextureFactor;
		DirectX::XMFLOAT4 toonTextureFactor;
	};

	struct GroupMorph
	{
		unsigned int morphIndex;
		float weight;
	};

	struct FlipMorph
	{
		unsigned int morphIndex;
		float weight;
	};

	struct ImpulseMorph
	{
		unsigned int rigidBodyIndex;
		unsigned char localFlag;
		DirectX::XMFLOAT3 translateVelocity;
		DirectX::XMFLOAT3 rotateTorque;
	};

	std::vector<PositionMorph> positionMorph;
	std::vector<UVMorph> uvMorph;
	std::vector<BoneMorph> boneMorph;
	std::vector<MaterialMorph> materialMorph;
	std::vector<GroupMorph> groupMorph;
	std::vector<FlipMorph> flipMorph;
	std::vector<ImpulseMorph> impulseMorph;
};

struct PMXIKLink
{
	unsigned int ikBoneIndex;
	unsigned char enableLimit;
	DirectX::XMFLOAT3 LimitMin;
	DirectX::XMFLOAT3 LimitMax;
};

struct PMXBone
{
	std::wstring name;
	std::string englishName;

	DirectX::XMFLOAT3 position;
	unsigned int parentBoneIndex;
	unsigned int deformDepth;

	PMXBoneFlags boneFlag;

	DirectX::XMFLOAT3 positionOffset;
	unsigned int linkBoneIndex;

	unsigned int appendBoneIndex;
	float appendWeight;

	DirectX::XMFLOAT3 fixedAxis;
	DirectX::XMFLOAT3 localXAxis;
	DirectX::XMFLOAT3 localZAxis;

	unsigned int keyValue;

	unsigned int ikTargetBoneIndex;
	unsigned int ikIterationCount;
	float ikLimit;

	std::vector<PMXIKLink> ikLinks;
};

class Wrapper;
class Camera;
class NodeManager;
class PmxModel : public Model
{
private:
	std::shared_ptr<Wrapper> _dx;
	std::shared_ptr<Camera> _camera;
	std::string _filePath;

	//ここからPMXファイルの読み込みに必要な構造体
	struct PMXHeader
	{
		std::array<unsigned char, 4 > magic;
		float version;
		unsigned char dataLength;
		unsigned char textEncoding;
		unsigned char addUVNum;
		unsigned char vertexIndexSize;
		unsigned char textureIndexSize;
		unsigned char materialIndexSize;
		unsigned char boneIndexSize;
		unsigned char morphIndexSize;
		unsigned char rigidBodyIndexSize;
	};

	struct PMXModelInfo
	{
		std::wstring modelName;
		std::string englishModelName;
		std::wstring comment;
		std::string englishComment;
	};

	struct PMXVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT4 additionalUV[4];
		PMXVertexWeight weightType;
		int boneIndices[4];
		float boneWeights[4];
		DirectX::XMFLOAT3 sdefC;
		DirectX::XMFLOAT3 sdefR0;
		DirectX::XMFLOAT3 sdefR1;

		float edgeMag;
	};

	struct PMXFace
	{
		int vertices[3];
	};

	struct PMXTexture
	{
		std::wstring textureName;
	};


	struct  PMXMaterial
	{
		std::wstring name;
		std::string englishName;

		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT3 specular;
		float specularPower;
		DirectX::XMFLOAT3 ambient;

		PMXDrawModeFlags drawMode;

		DirectX::XMFLOAT4 edgeColor;
		float edgeSize;

		unsigned int textureIndex;
		unsigned int sphereTextureIndex;
		PMXSphereMode sphereMode;

		PMXToonMode toonMode;
		unsigned int toonTextureIndex;

		std::wstring memo;

		unsigned int numFaceVertices;
	};

	

	struct PMXLoadedMaterial
	{
		bool visible;
		std::string name;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT3 specular;
		float specularPower;
		DirectX::XMFLOAT3 ambient;
		bool isTransparent;
	};

	std::vector< PMXLoadedMaterial >mLoadedMaterial;



	struct PMXDisplayFrame
	{
		std::wstring name;
		std::string englishName;

		enum class TargetType : uint8_t
		{
			BoneIndex,
			MorphIndex,
		};

		struct Target
		{
			TargetType type;
			unsigned int index;
		};

		enum class FrameType : uint8_t
		{
			DefaultFrame,
			SpecialFrame,
		};

		FrameType flag;
		std::vector<Target> targets;
	};

	struct PMXRigidBody
	{
		std::wstring name;
		std::string englishName;

		unsigned int boneIndex;
		unsigned char group;
		unsigned short collisionGroup;

		enum class Shape : uint8_t
		{
			Sphere,
			Box,
			Capsule,
		};

		Shape shape;
		DirectX::XMFLOAT3 shapeSize;
		DirectX::XMFLOAT3 translate;
		DirectX::XMFLOAT3 rotate;

		float mass;
		float translateDimmer;
		float rotateDimmer;
		float repulsion;
		float friction;

		enum class Operation : uint8_t
		{
			Static,
			Dynamic,
			DynamicAndBoneMerge,
		};
		Operation op;
	};

	struct PMXJoint
	{
		std::wstring name;
		std::string englishName;

		enum class JointType : uint8_t
		{
			SpringDOF6,
			DOF6,
			P2P,
			ConeTwist,
			Slider,
			Hinge,
		};
		JointType type;
		unsigned int rigidBodyAIndex;
		unsigned int rigidBodyBIndex;

		DirectX::XMFLOAT3 translate;
		DirectX::XMFLOAT3 rotate;

		DirectX::XMFLOAT3 translateLowerLimit;
		DirectX::XMFLOAT3 translateUpperLimit;
		DirectX::XMFLOAT3 rotateLowerLimit;
		DirectX::XMFLOAT3 rotateUpperLimit;

		DirectX::XMFLOAT3 springTranslateFactor;
		DirectX::XMFLOAT3 springRotateFactor;
	};

	struct PMXSoftBody
	{
		std::wstring name;
		std::string englishName;

		enum class SoftBodyType : uint8_t
		{
			TriMesh,
			Rope,
		};
		SoftBodyType type;

		unsigned int materialIndex;
		unsigned char group;
		unsigned short collisionGroup;

		enum class SoftBodyMask : uint8_t
		{
			BLink = 0x01,
			Cluster = 0x02,
			HybridLink = 0x04,
		};
		SoftBodyMask flag;

		unsigned int bLinkLength;
		unsigned int numClusters;

		float totalMass;
		float collisionMargin;

		enum class AeroModel : int32_t
		{
			kAeroModelV_TwoSided,
			kAeroModelV_OneSided,
			kAeroModelF_TwoSided,
			kAeroModelF_OneSided,
		};
		unsigned int aeroModel;

		float vcf;
		float dp;
		float dg;
		float lf;
		float pr;
		float vc;
		float df;
		float mt;
		float chr;
		float khr;
		float shr;
		float ahr;

		float srhr_cl;
		float skhr_cl;
		float sshr_cl;
		float sr_splt_cl;
		float sk_splt_cl;
		float ss_splt_cl;

		unsigned int v_it;
		unsigned int p_it;
		unsigned int d_it;
		unsigned int c_it;

		float lst;
		float ast;
		float vst;

		struct AnchorRigidBody
		{
			unsigned int rigidBodyIndex;
			unsigned int vertexIndex;
			unsigned char nearMode;
		};
		std::vector<AnchorRigidBody> anchorRigidBodies;

		std::vector<unsigned int> pinVertexIndices;
	};

	struct PMXFileData
	{
		PMXHeader header;
		PMXModelInfo modelInfo;

		std::vector<PMXVertex> vertices;
		std::vector<PMXFace> faces;
		std::vector<PMXTexture> textures;
		std::vector<PMXMaterial> materials;
		std::vector<PMXBone> bones;
		std::vector<PMXMorph> morphs;
		std::vector<PMXDisplayFrame> displayFrames;
		std::vector<PMXRigidBody> rigidBodies;
		std::vector <PMXJoint> joints;
		std::vector<PMXSoftBody> softBodies;
	};

	PMXFileData pmxData;
	//ここまで
	std::shared_ptr<NodeManager> _nodeManager;

	struct SkinningRange
	{
		unsigned int startIndex;
		unsigned int vertexCount;
	};

	

	bool Load(std::string filePath) override;
	bool ReadHeader(PMXFileData& data, std::ifstream& file);
	bool GetPMXStringUTF16(std::ifstream& _file, std::wstring& output);
	bool GetPMXStringUTF8(std::ifstream& _file, std::string& output);
	bool ReadModelInfo(PMXFileData& data, std::ifstream& file);
	bool ReadVertex(PMXFileData& data, std::ifstream& file);
	bool ReadFace(PMXFileData& data, std::ifstream& file);
	bool ReadTexture(PMXFileData& data, std::ifstream& file);
	bool ReadMaterial(PMXFileData& data, std::ifstream& file);
	bool ReadBone(PMXFileData& data, std::ifstream& file);
	bool ReadMorph(PMXFileData& data, std::ifstream& file);
	bool ReadDisplayFrame(PMXFileData& data, std::ifstream& file);
	bool ReadRigidBody(PMXFileData& data, std::ifstream& file);
	bool ReadJoint(PMXFileData& data, std::ifstream& file);
	bool ReadSoftBody(PMXFileData& data, std::ifstream& file);

	std::wstring GetTexturePathFromModelAndTexPath(const std::string& modelPath, const std::wstring& texPathW);

	unsigned int _duration = 0;
	DWORD _startTime;
	void InitAnimation(VMDFileData& vmdData);
	void PlayAnimation();
	void UpdateAnimation();
	void VertexSkinning();

	bool ModelHeapInit() override;
	void Draw() override;
	void Update() override;

	std::vector<SkinningRange> _skinningRanges;
	std::vector<std::future<void>> _parallelUpdateFutures;
	void InitParallelVertexSkinningSetting();
	void VertexSkinningByRange(const SkinningRange& range);

public:

	PmxModel(std::shared_ptr<Wrapper> dx
		, std::shared_ptr<Camera> camera,
		std::string filePath
	);
	~PmxModel() override;
};


