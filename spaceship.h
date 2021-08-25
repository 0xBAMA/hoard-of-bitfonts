#include "letters.h"
#include "vector.h"
#include <random>
#include <algorithm>

class ssGenerator {
public:
  std::vector<letter> glyphs;
  void populate() { // get the glyphs into the glyphs array
    // std::ifstream i("optimized.json");
    std::ifstream i("resources/hoard-of-bitfonts/optimized.json");
    json j; i >> j;
		for (auto& element : j) { // per character
			letter temp; int y = 0;
			temp.data.resize(element.size());
			for(auto& row : element) {
				temp.buildrow(y, row);
				y++;
			}
			if(!temp.nfg) glyphs.push_back(temp);
		}
    std::cout << "Loaded " << glyphs.size() << " glyphs\n";
  }

  int num_ops;

  int maxxyScale;
  int minxyScale;

  int maxzScale;
  int minzScale;


  struct bbox {
    ivec3 mins;
    ivec3 maxs;
  };

  struct col {
    vec3 rgb;
    float a;
  };

  struct vox {
    col color;
    ivec3 pos;
  };


  std::mt19937_64 rng;
  void genRandomEngine() {
    std::random_device r;
    std::seed_seq s{r(), r(), r(), r(), r(), r(), r(), r(), r()};
    rng = std::mt19937_64(s);
  }

  // iq style palette
  std::vector<col> pvals;
  void genPalette() {
    pvals.clear();
    std::uniform_real_distribution<float> gen(0.0618, 0.98);
    std::uniform_real_distribution<float> gena(0.618, 0.98);
    // get four random color values, to create a smooth palette
    for(unsigned int i = 0; i < 4; i++) {
      col c;
      c.rgb.values[0] = gen(rng);
      c.rgb.values[1] = gen(rng);
      c.rgb.values[2] = gen(rng);
      c.a = gena(rng);
      pvals.push_back(c);
    }
    // make sure one has a low alpha value, to allow for something like windows
    pvals[0].a = 0.08;
  }

  // sample the palette function at a randomly generated location
  // https://www.shadertoy.com/view/ll2GD3
  col genColor(float t) {
    col c;

    c.rgb.values[0] = pvals[0].rgb.values[0] + pvals[1].rgb.values[0] * cos(pi*2. * (pvals[2].rgb.values[0] * t + pvals[3].rgb.values[0]));
    c.rgb.values[1] = pvals[0].rgb.values[1] + pvals[1].rgb.values[1] * cos(pi*2. * (pvals[2].rgb.values[1] * t + pvals[3].rgb.values[1]));
    c.rgb.values[2] = pvals[0].rgb.values[2] + pvals[1].rgb.values[2] * cos(pi*2. * (pvals[2].rgb.values[2] * t + pvals[3].rgb.values[2]));
    c.a = pvals[0].a + pvals[1].a * cos(pi*2. * (pvals[2].a * t + pvals[3].a));

    return c;
  }




  // holds a list of occupied voxels and their content - this style of volume representation makes it easier to mirror, etc. than with a dense volume
  std::vector<vox> model;
  bbox getBBox() {
    // iterate through the model and keep the min and max valuex of x,y,z
    bbox temp;
    for(auto& m : model)
      for(unsigned int i = 0; i < 3; i++) {
        temp.maxs.values[i] = std::max(temp.maxs.values[i], m.pos.values[i]);
        temp.mins.values[i] = std::min(temp.mins.values[i], m.pos.values[i]);
      }
    return temp;
  }

  void addVox(vox v) {
    // check contents -
    for(auto& m : model) {
      // if previously occupied, change color for model's entry
      if( m.pos == v.pos ) {
        m.color = v.color;
        return;
      }
    }
    // if not previously occupied, push v onto the model
    model.push_back(v);
  }

  void stampRandom() { // stamping into the model list
    // pick a random glyph in the list
    std::uniform_int_distribution<int> pick(0, glyphs.size()-1);
    letter l = glyphs[pick(rng)];

    // stamp parameters:
    // xy scale
    std::uniform_int_distribution<int> xyScaleG(minxyScale, maxxyScale);
    int xyScale = xyScaleG(rng);

    // z scale
    std::uniform_int_distribution<int> zScaleG(minzScale, maxzScale);
    int zScale = zScaleG(rng);

    // RGBA value
    std::uniform_real_distribution<float> tG(0., 1.);
    float t = tG(rng);
    col c = genColor(t);

    // orientation
    std::uniform_int_distribution<int> orientG(0, 2);
    int orientation = orientG(rng);

    // where to draw? base point will be somewhere in the current bounding box
    bbox b = getBBox();
    std::uniform_int_distribution<int> xG(b.mins.values[0], b.maxs.values[0]);
    std::uniform_int_distribution<int> yG(b.mins.values[1], b.maxs.values[1]);
    std::uniform_int_distribution<int> zG(b.mins.values[2], b.maxs.values[2]);
    ivec3 base = ivec3(xG(rng), yG(rng), zG(rng));

    // stamp it (overwrite any existing contents)
    vox v; v.color = c;
		for(unsigned int xx = 0; xx < l.data.size(); xx++)
		for(unsigned int yy = 0; yy < l.data[0].size(); yy++) {
      if(l.data[xx][yy]==1)
        for(int xs = 0; xs < xyScale; xs++)
        for(int ys = 0; ys < xyScale; ys++)
        for(int zs = 0; zs < zScale; zs++) {
          switch (orientation) {
            // the drawing will start at the specified base point
            case 0: v.pos = base + ivec3(xs, ys, zs) + ivec3(xx*xyScale, yy*xyScale, 0); break;
            case 1: v.pos = base + ivec3(ys, xs, zs) + ivec3(yy*xyScale, xx*xyScale, 0); break;
            case 2: v.pos = base + ivec3(ys, zs, xs) + ivec3(yy*xyScale, 0, xx*xyScale); break;
            default: break;
          }
          addVox(v);
        }
    }
  }

  void flip() {
    // pick an axis, and subtract the bbox axis max from the
    squareModel();
    std::uniform_int_distribution<int> axisPick(0, 2);
    int axis = axisPick(rng);
    bbox b = getBBox();

    for(unsigned int i = 0; i < model.size(); i++) {
      model[i].pos.values[axis] = b.maxs.values[axis] - model[i].pos.values[axis];
    }
  }

  void mirror() {
    // pick an axis, then create a copy of all vox structs mirrored about 0
    squareModel();
    std::uniform_int_distribution<int> axisPick(0, 2);
    int axis = axisPick(rng);

    for(unsigned int i = 0; i < model.size(); i++) {
      vox v = model[i];
      v.pos.values[axis] *= -1;
      addVox(v);
    }
  }

  void squareModel() {
    // makes sure all indices are positive offsets from negative faces
    bbox b = getBBox();
    for(auto& m : model) {
      m.pos.values[0] -= b.mins.values[0];
      m.pos.values[1] -= b.mins.values[1];
      m.pos.values[2] -= b.mins.values[2];
    }
  }

  void genSpaceship() {
    model.clear();
    // std::uniform_int_distribution<int> opPick(0, 3);
    //
    // for(unsigned int i = 0; i < num_ops; i++) {
    //   std::cout << "operation " << i << std::endl;
    //   switch(opPick(rng)){
    //     case 0:
    //     case 1:
    //       for(unsigned int i = 0; i < 10; i++)
    //         stampRandom();
    //       break;
    //
    //     case 2:
    //       mirror(); flip();
    //       mirror();
    //       break;
    //
    //     case 3:
    //       mirror();
    //       break;
    //
    //     default:
    //       break;
    //   }
    // }

    for(unsigned int i = 0; i < 18; i++)
      stampRandom();
    mirror();
    flip();
    mirror();
    mirror();
    mirror();


  }

  void dump() {
    // make sure all indices are positive
    squareModel();

    // generate dense 3D volume and write slices to CLI
    bbox b = getBBox(); // get bounding box and get side lengths
    std::vector<unsigned char> values; // make a block this size and init w/ 0s
    values.resize(b.maxs.values[0]*b.maxs.values[1]*b.maxs.values[2], 0);

    // populate the vector of values
    for(auto& m : model) {
      int p = int(m.pos.values[0])+int(m.pos.values[1]*b.maxs.values[0])+int(m.pos.values[2]*b.maxs.values[0]*b.maxs.values[1]);
      values[p] = 1;
    }

    std::cout << "otuput block is " << int(b.maxs.values[0]) << "x "<< int(b.maxs.values[1]) << "y "<< int(b.maxs.values[2]) << "z\n";

    // debug view while prototyping, eventually this will be a Voraldo feature
    for(int z = 0; z < int(b.maxs.values[2]); z++) {
    for(int y = 0; y < int(b.maxs.values[1]); y++) {
    for(int x = 0; x < int(b.maxs.values[0]); x++) {
      bool v = (values[x+int(y*b.maxs.values[0])+int(z*b.maxs.values[0]*b.maxs.values[1])] == 1);
      std::cout << v ? 1 : 0;
    } std::cout << std::endl << std::flush;
    } std::cout << std::endl;
    }
  }


  void getData(std::vector<unsigned char> &data, int dim) {
    if(data.size()==0) return;

    // put it in the center
    bbox b = getBBox();
    ivec3 offset = ivec3((b.mins.values[0]+b.maxs.values[0])/2, (b.mins.values[1]+b.maxs.values[1])/2, (b.mins.values[2]+b.maxs.values[2])/2);

    for(unsigned int i = 0; i < model.size(); i++) {
      model[i].pos -= offset;
      model[i].pos += ivec3(dim/2);
      if(model[i].pos.values[0] < 0 || model[i].pos.values[0] >= dim || model[i].pos.values[1] < 0 || model[i].pos.values[1] >= dim || model[i].pos.values[2] < 0 || model[i].pos.values[2] >= dim) {
        model.erase(model.begin()+i);
        i--;
      }
    }

    for(unsigned int i = 0; i < model.size(); i++) {
      vox current = model[i];
      int index = 4 * (current.pos.values[0] + current.pos.values[1]*dim + current.pos.values[2]*dim*dim);
      data[index+0] = current.color.rgb.values[0] * 255;
      data[index+1] = current.color.rgb.values[1] * 255;
      data[index+2] = current.color.rgb.values[2] * 255;
      data[index+3] = current.color.a * 255;
    }




  }
};
