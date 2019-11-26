//==============================================================================
//
//  TranscodeApplication
//
//  Created by Kwon Keuk Han
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================

#include <iostream>
#include <unistd.h>

#include "transcode_application.h"

#define OV_LOG_TAG "TranscodeApplication"

std::shared_ptr<TranscodeApplication> TranscodeApplication::Create(const info::Application &application_info)
{
	auto instance = std::make_shared<TranscodeApplication>(application_info);

	return instance;
}

TranscodeApplication::TranscodeApplication(const info::Application &application_info)
	: _application_info(application_info)
{
	logtd("Transcode application [%s] is created", _application_info.GetName().CStr());
}

TranscodeApplication::~TranscodeApplication()
{
	logtd("Destroyed transcode application.");
}

bool TranscodeApplication::OnCreateStream(const std::shared_ptr<StreamInfo> &stream_info)
{
	logtd("OnCreateStream (%s)", stream_info->GetName().CStr());

	std::unique_lock<std::mutex> lock(_mutex);

	auto stream = std::make_shared<TranscodeStream>(_application_info, stream_info, this);

	_streams.insert(std::make_pair(stream_info->GetId(), stream));

	return true;
}

bool TranscodeApplication::OnDeleteStream(const std::shared_ptr<StreamInfo> &stream_info)
{
	logtd("OnDeleteStream (%s)", stream_info->GetName().CStr());

	std::unique_lock<std::mutex> lock(_mutex);

	auto stream_bucket = _streams.find(stream_info->GetId());

	if(stream_bucket == _streams.end())
	{
		return false;
	}

	auto stream = stream_bucket->second;

	stream->Stop();

	_streams.erase(stream_info->GetId());

	return true;
}


bool TranscodeApplication::OnSendFrame(const std::shared_ptr<StreamInfo> &stream_info, const std::shared_ptr<MediaPacket> &packet)
{
	std::unique_lock<std::mutex> lock(_mutex);

	auto stream_bucket = _streams.find(stream_info->GetId());

	if(stream_bucket == _streams.end())
	{
		return false;
	}

	auto stream = stream_bucket->second;

	return stream->Push(std::move(packet));
}
