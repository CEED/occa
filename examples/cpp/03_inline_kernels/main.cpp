#include <iostream>

#include <occa.hpp>

occa::json parseArgs(int argc, const char **argv);

int main(int argc, const char **argv) {
  occa::json args = parseArgs(argc, argv);

  occa::setDevice((std::string) args["options/device"]);

  int entries = 5;

  float *a  = new float[entries];
  float *b  = new float[entries];
  float *ab = new float[entries];

  for (int i = 0; i < entries; ++i) {
    a[i]  = i;
    b[i]  = 1 - i;
    ab[i] = 0;
  }

  occa::memory o_a  = occa::malloc(entries, occa::dtype::float_, a);
  occa::memory o_b  = occa::malloc(entries, occa::dtype::float_, b);
  occa::memory o_ab = occa::malloc(entries, occa::dtype::float_);

  // Props are
  occa::properties props;
  props["defines/TILE_SIZE"] = 16;

  // OCCA_INLINED_KERNEL Argument Format:
  //   - First argument:
  //       - Variables used in the kernel wrapped in ()'s
  //
  //   - Second argument:
  //       - Props for the kernel
  //       - Pass occa::properties() if no props are needed
  //
  //   - Third argument:
  //       - Kernel body wrapped in ()'s
  //
  // Restrictions
  //   - Memory allocations must include a dtype
  //       - To build the kernel at runtime, the types have to be known
  //
  // Temporary Restrictions:
  //   - Memory objects must always be of the same dtype
  //       - Resolved once 'auto' is supported. Function arguments of
  //         type 'auto' will act as templated typed variables
  //
  //   - Cannot use unified memory from occa::umalloc
  //       - dtype::get<> needs to check a pointer type
  //
  //   ~ Cannot use external functions
  //       - Potentially can happen with another macro OCCA_INLINED_FUNCTION
  OCCA_INLINED_KERNEL(
    (entries, o_a, o_b, o_ab),
    props,
    (
      for (int i = 0; i < entries; ++i; @tile(TILE_SIZE, @outer, @inner)) {
        o_ab[i] = o_a[i] + o_b[i];
      }
    )
  );

  // Copy result to the host
  o_ab.copyTo(ab);

  for (int i = 0; i < 5; ++i)
    std::cout << i << ": " << ab[i] << '\n';

  for (int i = 0; i < entries; ++i) {
    if (ab[i] != (a[i] + b[i]))
      throw 1;
  }

  delete [] a;
  delete [] b;
  delete [] ab;

  return 0;
}

occa::json parseArgs(int argc, const char **argv) {
  // Note:
  //   occa::cli is not supported yet, please don't rely on it
  //   outside of the occa examples
  occa::cli::parser parser;
  parser
    .withDescription(
      "Example showing how to use background devices, allowing passing of the device implicitly"
    )
    .addOption(
      occa::cli::option('d', "device",
                        "Device properties (default: \"mode: 'Serial'\")")
      .withArg()
      .withDefaultValue("mode: 'Serial'")
    )
    .addOption(
      occa::cli::option('v', "verbose",
                        "Compile kernels in verbose mode")
    );

  occa::json args = parser.parseArgs(argc, argv);
  occa::settings()["kernel/verbose"] = args["options/verbose"];

  return args;
}