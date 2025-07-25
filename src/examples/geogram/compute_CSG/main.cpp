/*
 *  Copyright (c) 2000-2022 Inria
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#include <geogram/basic/common.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/mesh/mesh_CSG.h>
#include <geogram/mesh/mesh_io.h>

namespace {

    void configure_builder(GEO::CSGBuilder& builder) {
	builder.set_simplify_coplanar_facets(
	    GEO::CmdLine::get_arg_bool("simplify_coplanar_facets"),
	    GEO::CmdLine::get_arg_double("coplanar_angle_tolerance")
	);
	builder.set_delaunay(GEO::CmdLine::get_arg_bool("delaunay"));
	builder.set_detect_intersecting_neighbors(
	    GEO::CmdLine::get_arg_bool("detect_intersecting_neighbors")
	);
	builder.set_fast_union(GEO::CmdLine::get_arg_bool("fast_union"));
	builder.set_noop(GEO::CmdLine::get_arg_bool("noop"));
	if(GEO::CmdLine::get_arg_bool("clear_cache")) {
	    GEOCSG::OpenSCAD_cache_invalidate();
	}
	if(GEO::CmdLine::get_arg_bool("ignore_cache_time")) {
	    GEOCSG::OpenSCAD_cache_ignore_time();
	}
    }

    std::shared_ptr<GEO::Mesh> example001() {
        using namespace GEO;
        CSGBuilder B;
	configure_builder(B);
        return B.difference({
                B.sphere(25.0),
                B.multmatrix(
                    {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
                    { B.cylinder(62.5, 12.5, 12.5) }
                ),
                B.multmatrix(
                    {{1, 0, 0, 0}, {0, 0, -1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
                    { B.cylinder(62.5, 12.5, 12.5) }
                ),
                B.multmatrix(
                    {{0, 0, 1, 0}, {0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
                    { B.cylinder(62.5, 12.5, 12.5) }
                )
            });
    }

    std::shared_ptr<GEO::Mesh> example002() {
        using namespace GEO;
        CSGBuilder B;
	configure_builder(B);
        return B.intersection({
                B.difference({
                        B.union_instr({
                                B.cube({30,30,30}),
                                B.multmatrix(
                                    {{1, 0, 0,  0},
                                     {0, 1, 0,  0},
                                     {0, 0, 1, -25},
                                     {0, 0, 0, 1}},
                                    {B.cube({15,15,40})}
                                )
                            }),
                        B.union_instr({
                                B.cube({50,10,10}),
                                B.cube({10,50,10}),
                                B.cube({10,10,50})
                            }),
                    }),
                B.multmatrix(
                    {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 5}, {0, 0, 0, 1}},
                    { B.cylinder(50, 20, 5) }
                )
            });
    }

    std::shared_ptr<GEO::Mesh> example003() {
        using namespace GEO;
        CSGBuilder B;
	configure_builder(B);
        return B.difference({
                B.union_instr({
                        B.cube({30, 30, 30}),
                        B.cube({40, 15, 15}),
                        B.cube({15, 40, 15}),
                        B.cube({15, 15, 40})
                    }),
                B.union_instr({
                        B.cube({50, 10, 10}),
                        B.cube({10, 50, 10}),
                        B.cube({10, 10, 50})
                    })
            });
    }

    std::shared_ptr<GEO::Mesh> example004() {
        using namespace GEO;
        CSGBuilder B;
	configure_builder(B);
        return B.difference({
                B.cube({30,30,30}),
                B.sphere(20)
            });
    }
}

int main(int argc, char** argv) {
    using namespace GEO;
    try {
        GEO::initialize(GEO::GEOGRAM_INSTALL_ALL);

        CmdLine::import_arg_group("standard");
        CmdLine::import_arg_group("algo");

        std::vector<std::string> filenames;

        CmdLine::declare_arg(
            "verbose",false,"makes intersection algorithm more chatty"
        );

        CmdLine::declare_arg(
            "fine_verbose",false,"makes intersection algorithm even more chatty"
        );

        CmdLine::declare_arg(
            "simplify_coplanar_facets",true,
            "simplify coplanar facets whenever possible"
        );

        CmdLine::declare_arg(
            "coplanar_angle_tolerance",0.0,
            "maximum angle (in degrees) between coplanar facets"
        );

        CmdLine::declare_arg(
            "delaunay",true, "use Delaunay triangulation (nice triangles)"
        );

        CmdLine::declare_arg(
            "detect_intersecting_neighbors",true,
            "detect intersecting neighbors in input and intermediary meshes"
        );

        CmdLine::declare_arg(
            "fast_union", true,
            "fast union mode (there is no cnx component completely inside)"
        );

        CmdLine::declare_arg(
            "noop",false,
            "replace union, intersection, difference with append"
        );

	CmdLine::declare_arg(
	    "clear_cache", false,
	    "systematically regenerate files converted with OpenSCAD"
	);

	GEO::CmdLine::declare_arg(
	    "ignore_cache_time", false,
	    "ignore file modification time for deciding to use cache"
	);

        if(
            !CmdLine::parse(
                argc, argv, filenames, "csgfilename <outputfile|none>"
            )
        ) {
            return 1;
        }

        Stopwatch Wtot("CSG (total)");

        std::string csg_filename = filenames[0];

        std::string output_filename =
            filenames.size() >= 2 ? filenames[1] : std::string("out.meshb");

	std::shared_ptr<Mesh> result;

        if(csg_filename == "example001") {
            result = example001();
        } else if(csg_filename == "example002") {
            result = example002();
        } else if(csg_filename == "example003") {
            result = example003();
        } else if(csg_filename == "example004") {
            result = example004();
	} else {
            CSGCompiler CSG;
	    configure_builder(CSG.builder());
            CSG.set_verbose(CmdLine::get_arg_bool("verbose"));
	    CSG.set_fine_verbose(CmdLine::get_arg_bool("fine_verbose"));
            result = CSG.compile_file(csg_filename);
        }
        if(result == nullptr) {
            Logger::err("CSG") << "No output (problem occured)" << std::endl;
            return 2;
        } else {
            mesh_save(*result, output_filename);
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Received an exception: " << e.what() << std::endl;
        return 1;
    }

    Logger::out("") << "Everything OK, Returning status 0" << std::endl;
    return 0;
}
