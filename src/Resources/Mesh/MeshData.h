#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <string>
#include <boost/array.hpp>
#include "Math.h"
#include "StdTypes.h"
#include "Vec.h"


/// Mesh data. This class loads the mesh file and the Mesh class loads it to the GPU
///
/// Binary file format:
///
/// @code
/// <magic:ANKIMESH>
/// <string:meshName>
/// <uint:vertsNum>
/// <float:vert[0].x> <float:vert[0].y> <float:vert[0].z> ...
/// <float:vert[vertsNum-1].x> <float:vert[vertsNum-1].y> <float:vert[vertsNum-1].z>
/// <uint:facesNum>
/// <uint:tri[0].vertIds[0]> <uint:tri[0].vertIds[1]> <uint:tri[0].vertIds[2]> ...
/// <uint:tri[facesNum-1].vertIds[0]> <uint:tri[facesNum-1].vertIds[1]> <uint:tri[facesNum-1].vertIds[2]>
/// <uint:texCoordsNum>
/// <float:texCoord[0].x><float:texCoord[0].y> ...
/// <float:texCoord[texCoordsNum-1].x><float:texCoord[texCoordsNum-1].y>
/// @endcode
class MeshData
{
	public:
		/// Vertex weight for skeletal animation
		class VertexWeight
		{
			public:
				/// Dont change this or prepare to change the skinning code in shader
				static const uint MAX_BONES_PER_VERT = 4;

				/// @todo change the vals to uint when change drivers
				float bonesNum;
				boost::array<float, MAX_BONES_PER_VERT> boneIds;
				boost::array<float, MAX_BONES_PER_VERT> weights;
		};

		/// Triangle
		class Triangle
		{
			public:
				uint vertIds[3]; ///< an array with the vertex indexes in the mesh class
				Vec3 normal;
		};

		MeshData(const char* filename) {load(filename);}
		~MeshData() {}

		/// @name Accessors
		/// @{
		const Vec<Vec3>& getVertCoords() const {return vertCoords;}
		const Vec<Vec3>& getVertNormals() const {return vertNormals;}
		const Vec<Vec4>& getVertTangents() const {return vertTangents;}
		const Vec<Vec2>& getTexCoords() const {return texCoords;}
		const Vec<VertexWeight>& getVertWeights() const {return vertWeights;}
		const Vec<Triangle>& getTris() const {return tris;}
		const Vec<ushort>& getVertIndeces() const {return vertIndeces;}
		/// @}

	private:
		/// @name Data
		/// @{
		Vec<Vec3> vertCoords; ///< Loaded from file
		Vec<Vec3> vertNormals; ///< Generated
		Vec<Vec4> vertTangents; ///< Generated
		Vec<Vec2> texCoords; ///< Optional. One for every vert so we can use vertex arrays & VBOs
		Vec<VertexWeight> vertWeights; ///< Optional
		Vec<Triangle> tris; ///< Required
		Vec<ushort> vertIndeces; ///< Generated. Used for vertex arrays & VBOs
		/// @}

		/// Load the mesh data from a binary file
		/// @exception Exception
		void load(const char* filename);

		void createFaceNormals();
		void createVertNormals();
		void createAllNormals();
		void createVertTangents();
		void createVertIndeces();

		/// This func does some sanity checks and creates normals, tangents, VBOs etc
		/// @exception Exception
		void doPostLoad();
};


inline void MeshData::createAllNormals()
{
	createFaceNormals();
	createVertNormals();
}


#endif