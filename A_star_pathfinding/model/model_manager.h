#ifndef MODEL_MANAGER_H_
#define MODEL_MANAGER_H_

using namespace std;

#include <unordered_map>
#include "model_enum.h"

#include "utility.h"

#include "model_constant.h"
#include "model.h"
#include "quad_model.h"
#include "xyz_axis_model.h"


#include "json_spirit.h"
#include "json_spirit_reader_template.h"
#include "json_spirit_writer_template.h"
#include <cassert>
#include <fstream>

#ifndef JSON_SPIRIT_MVALUE_ENABLED
#error Please define JSON_SPIRIT_MVALUE_ENABLED for the mValue type to be enabled 
#endif

using namespace std;
using namespace json_spirit;

// to resolve circular depenency issue
// class Weapon;

class ModelManager
{


	public:

		ModelManager();
		~ModelManager();

		void init();
		void shutDown();

		Model* get(int modelEnum);


		static void enableVertexAttribArrays();
		static void disableVertexAttribArrays();

		static void buildLinesBetweenTwoPoints2D(glm::vec2 p0, glm::vec2 p1, float thickness,
			vector<VertexData>& vertices,
			vector<unsigned int>& indices);

		static void buildLinesBetweenTwoPoints2D(glm::vec3 p0, glm::vec3 p1, float thickness,
			vector<VertexData>& vertices,
			vector<unsigned int>& indices);

		static void buildCircle(float radius, float thickness, vector<VertexData>& vertices, vector<unsigned int>& indices);
		
		
		static void buildTriangleForCircleSlice2D(glm::vec2 p0, glm::vec2 p1, glm::vec2 center,
			vector<VertexData>& vertices,
			vector<unsigned int>& indices);	

		static void buildTriangleForCircleSlice2D(glm::vec3 p0, glm::vec3 p1, glm::vec3 center,
			vector<VertexData>& vertices,
			vector<unsigned int>& indices);
		static void buildFilledCircle(float radius, vector<VertexData>& vertices, vector<unsigned int>& indices);
		
	private:
		Model* m_quad;
		Model* m_centeredQuad;
		Model* m_xyzAxis;

		vector<Model*> m_models;
};



#endif