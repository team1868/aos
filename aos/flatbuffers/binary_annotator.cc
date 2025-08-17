#include "aos/flatbuffers/binary_annotator.h"

#include <span>

#include "aos/json_to_flatbuffer.h"
#include "src/annotated_binary_text_gen.h"
#include "src/binary_annotator.h"

namespace aos::fbs {
std::string AnnotateBinaries(
    const aos::NonSizePrefixedFlatbuffer<reflection::Schema> &schema,
    flatbuffers::span<const uint8_t> binary_data) {
  flatbuffers::BinaryAnnotator binary_annotator(
      schema.span().data(), schema.span().size(), binary_data.data(),
      binary_data.size(), /*is_size_prefixed=*/false);

  auto annotations = binary_annotator.Annotate();

  flatbuffers::AnnotatedBinaryTextGenerator text_generator(
      flatbuffers::AnnotatedBinaryTextGenerator::Options{}, annotations,
      binary_data.data(), binary_data.size());

  return text_generator.GenerateString();
}

std::string AnnotateBinaries(const std::filesystem::path &schema_bfbs_file,
                             flatbuffers::span<const uint8_t> binary_data) {
  return AnnotateBinaries(
      aos::FileToFlatbuffer<reflection::Schema>(schema_bfbs_file.string()),
      binary_data);
}
}  // namespace aos::fbs
