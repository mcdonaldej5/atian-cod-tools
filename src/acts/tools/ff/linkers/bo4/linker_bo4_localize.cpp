#include <includes.hpp>
#include <tools/ff/linkers/linker_bo4.hpp>

namespace fastfile::linker::bo4 {
	class LocalizeWorker : public LinkerWorker {
	public:
		LocalizeWorker() : LinkerWorker("Localize") {}

		void Compute(BO4LinkContext& ctx) override {

			std::filesystem::path localizePath{ ctx.linkCtx.input / "localize.json" };
			core::config::Config localize{ localizePath };
			localize.SyncConfig(false);
			if (localize.main.Empty()) return; // nothing to force

			struct LocalizeEntry {
				const char* value;
				XHash name;
			};
			static_assert(sizeof(LocalizeEntry) == 0x18);

			size_t added{};
			for (auto& [k, v] : localize.main.GetObj()) {
				if (!k.IsString() || !v.IsString()) {
					LOG_WARNING("Invalid localize: {} -> {}", k.GetString(), v.GetString());
					continue;
				}
				ctx.data.AddAsset(games::bo4::pool::ASSET_TYPE_LOCALIZE_ENTRY, fastfile::linker::data::POINTER_NEXT);

				ctx.data.PushStream(XFILE_BLOCK_TEMP);
				LocalizeEntry header{};

				header.name.name = hash::Hash64Pattern(k.GetString());
				header.value = (const char*)fastfile::linker::data::POINTER_NEXT;
				ctx.data.WriteData(header);

				ctx.data.PushStream(XFILE_BLOCK_VIRTUAL);
				ctx.data.Align<char>();
				ctx.data.WriteData(v.GetString());
				ctx.data.PopStream();

				ctx.data.PopStream();
				added++;
			}
			LOG_INFO("Added asset localizeentry {}: {} entry(ies)", localizePath.string(), added);
		}
	};

	utils::ArrayAdder<LocalizeWorker, LinkerWorker> impl{ GetWorkers() };
}