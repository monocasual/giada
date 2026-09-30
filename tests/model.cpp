#include "src/core/model/model.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("Model", "[model]")
{
	using namespace giada;
	using namespace giada::m::model;

	Model model;
	const ID waveId{42};

	SECTION("single-part writes see earlier writes and only notify their own part")
	{
		bool assetsSawDocument = false;
		bool assetsSawEdit = false;
		bool documentSawAssets = false;

		model.write(SwapType::SOFT, [](DocumentEdit& edit)
		{
			edit.get().kernelAudio.samplerate = 12345;
		});
		model.flush();
		REQUIRE(model.pollDocumentChanges() == SwapType::SOFT);
		REQUIRE(model.pollAssetsChanges() == SwapType::NONE);

		model.write(SwapType::HARD, [&](AssetsEdit& edit)
		{
			assetsSawDocument = edit.view().getDocument().kernelAudio.samplerate == 12345;
			edit.get().waves.set(waveId, std::make_shared<model::Wave>(waveId));
			assetsSawEdit = edit.view().getAssets().waves.find(waveId) != nullptr;
		});
		model.write(SwapType::SOFT, [&](DocumentEdit& edit)
		{
			edit.get().kernelAudio.samplerate = 23456;
			documentSawAssets = edit.view().getAssets().waves.find(waveId) != nullptr &&
			    edit.view().getDocument().kernelAudio.samplerate == 23456;
		});
		model.flush();

		REQUIRE(assetsSawDocument);
		REQUIRE(assetsSawEdit);
		REQUIRE(documentSawAssets);
		REQUIRE(model.pollDocumentChanges() == SwapType::SOFT);
		REQUIRE(model.pollAssetsChanges() == SwapType::HARD);
	}

	SECTION("combined writes and owning reads")
	{
		model.write(SwapType::SOFT, [&](ModelEdit& edit)
		{
			edit.getDocument().kernelAudio.samplerate = 12345;
			edit.getAssets().waves.set(waveId, std::make_shared<model::Wave>(waveId));
			edit.getParameters().mixer.setActive(true);
		});
		model.flush();

		const auto oldView = [&model]()
		{
			const auto view = model.read();
			const auto copy = view;
			return copy;
		}();
		model.write(SwapType::HARD, [&](ModelEdit& edit)
		{
			edit.getDocument().kernelAudio.samplerate = 23456;
			edit.getAssets().waves.remove(waveId);
		});
		model.flush();

		const auto newView = model.read();
		REQUIRE(oldView.getDocument().kernelAudio.samplerate == 12345);
		REQUIRE(oldView.getAssets().waves.find(waveId) != nullptr);
		REQUIRE(newView.getDocument().kernelAudio.samplerate == 23456);
		REQUIRE(newView.getAssets().waves.find(waveId) == nullptr);
		REQUIRE(newView.getParameters().mixer.isActive());
		REQUIRE(model.pollDocumentChanges() == SwapType::HARD);
		REQUIRE(model.pollAssetsChanges() == SwapType::HARD);
	}

	SECTION("realtime read sees published changes")
	{
		model.write(SwapType::NONE, [](DocumentEdit& edit)
		{
			edit.get().kernelAudio.samplerate = 12345;
		});
		model.flush();

		const auto view = model.readRealtime();
		REQUIRE(view.getDocument().kernelAudio.samplerate == 12345);
		REQUIRE_FALSE(view.getParameters().mixer.isActive());
		REQUIRE(model.pollDocumentChanges() == SwapType::NONE);
	}
}
