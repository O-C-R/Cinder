/*
 Copyright (c) 2015, The Cinder Project: http://libcinder.org
 All rights reserved.
 
 Portions Copyright (c) 2010, Hector Sanchez-Pajares
 Aer Studio http://www.aerstudio.com
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "boost/signals2/signal.hpp"
#include "Osc.h"

namespace cinder { namespace tuio {

namespace detail {
template<typename VEC_T>
class Cursor;

template<typename VEC_T, typename ROT_T>
class Object;

template<typename VEC_T, typename ROT_T, typename DIM_T>
class Blob;
	
class Blob2D;
class Blob25D;
class Blob3D;
	
class ProfileHandlerBase;
}

using Cursor2D	= detail::Cursor<ci::vec2>;
using Cursor25D = detail::Cursor<ci::vec3>;
using Cursor3D	= detail::Cursor<ci::vec3>;
	
using Object2D	= detail::Object<ci::vec2, float>;
using Object25D = detail::Object<ci::vec3, float>;
using Object3D	= detail::Object<ci::vec3, ci::vec3>;
	
using Blob2D = detail::Blob2D;
using Blob25D = detail::Blob25D;
using Blob3D = detail::Blob3D;
	

//! Implements a Listener for the TUIO 1.1 protocol, described here: http://www.tuio.org/?specification
class Listener {
public:
	template<typename Profile>
	using ProfileFn = std::function<void(const Profile&)>;
	
	Listener( const app::WindowRef &window,
		    uint16_t localPort = DEFAULT_TUIO_PORT,
		    const asio::ip::udp &protocol = asio::ip::udp::v4(),
		    asio::io_service &io = ci::app::App::get()->io_service() );
	Listener( const osc::ReceiverBase *ptr );
	
	void bind();
	void listen();
	void close();
	
	//! Registers an async callback which fires when a new cursor is added
	template<typename CallbackFn>
	void	setProfileAddedCallback( CallbackFn callback );
	//! Registers an async callback which fires when a cursor is updated
	template<typename CallbackFn>
	void	setProfileUpdatedCallback( CallbackFn callback );
	//! Registers an async callback which fires when a cursor is removed
	template<typename CallbackFn>
	void	setProfileRemovedCallback( CallbackFn callback );
	
	//! Returns a std::vector of all active touches, derived from \c 2Dcur (Cursor) messages
	template<typename ProfileType>
	std::vector<ProfileType> getActiveProfiles() const;
	
	//! Returns the threshold for a frame ID being old enough to imply a new source
	int32_t	getPastFrameThreshold() const;
	//! Sets the threshold for a frame ID being old enough to imply a new source
	void	setPastFrameThreshold( int32_t pastFrameThreshold );
	
	static const int DEFAULT_TUIO_PORT = 3333;
	// default threshold for a frame ID being old enough to imply a new source
	static const int32_t DEFAULT_PAST_FRAME_THRESHOLD = 10;
	
private:
	template<typename T>
	static const char* getOscAddressFromType();
	
	using ProfileHandlers = std::map<std::string, std::unique_ptr<detail::ProfileHandlerBase>>;
	
	std::unique_ptr<osc::ReceiverBase>	mListener;
	ProfileHandlers						mHandlers;
	app::WindowRef						mWindow;
};

namespace detail {
	
struct Profile {
	
	int32_t getSessionId() const { return mSessionId; }
	const std::string& getSource() const { return mSource; }
	void setSource( const std::string &source ) { mSource = source; }
	
protected:
	Profile( const osc::Message &msg );
	Profile( const Profile &other ) = default;
	Profile( Profile &&other ) = default;
	Profile& operator=( const Profile &other ) = default;
	Profile& operator=( Profile &&other ) = default;
	~Profile() = default;
	
	int32_t		mSessionId;
	std::string mSource;
};

template<typename T>
struct ProfileCompare {
	bool operator()( const T& lhs, const T& rhs ) const
	{
		return lhs.getSource() < rhs.getSource() &&
		lhs.getSessionId() < rhs.getSessionId();
	}
};

template<typename VEC_T>
struct Cursor : public Profile {
	Cursor( const osc::Message &msg );
	
	Cursor( const Cursor &other ) = default;
	Cursor( Cursor &&other ) = default;
	Cursor& operator=( const Cursor &other ) = default;
	Cursor& operator=( Cursor &&other ) = default;
	~Cursor() = default;
	
	const VEC_T&	getPosition() const { return mPosition; }
	const VEC_T&	getVelocity() const { return mVelocity; }
	float			getAcceleration() const { return mAcceleration; }
	
	app::TouchEvent::Touch	convertToTouch() const;
	
protected:
	VEC_T		mPosition,
	mVelocity;
	float		mAcceleration;
};

template<typename VEC_T, typename ROT_T>
struct Object : public Profile {
	Object( const osc::Message &msg );
	
	Object( const Object &other ) = default;
	Object( Object &&other ) = default;
	Object& operator=( const Object &other ) = default;
	Object& operator=( Object &&other ) = default;
	~Object() = default;
	
	int32_t			getClassId() const { return mClassId; }
	const VEC_T&	getPosition() const { return mPosition; }
	const VEC_T&	getVelocity() const { return mVelocity; }
	const ROT_T&	getAngle() const { return mAngle; }
	const ROT_T&	getRotationVelocity() const { return mRotationVelocity; }
	float			getAcceleration() const { return mAcceleration; }
	float			getRotationAcceleration() const { return mRotateAccel; }
protected:
	int32_t		mClassId;
	VEC_T		mPosition, mVelocity;
	ROT_T		mAngle, mRotationVelocity;
	float		mAcceleration, mRotateAccel;
};

template<typename VEC_T, typename ROT_T, typename DIM_T>
struct Blob : public Profile {
	Blob( const osc::Message &msg );
	
	Blob( const Blob &other ) = default;
	Blob( Blob &&other ) = default;
	Blob& operator=( const Blob &other ) = default;
	Blob& operator=( Blob &&other ) = default;
	~Blob() = default;
	
	const VEC_T&	getPosition() const { return mPosition; }
	const VEC_T&	getVelocity() const { return mVelocity; }
	const ROT_T&	getAngle() const { return mAngle; }
	const ROT_T&	getRotationVelocity() const { return mRotationVelocity; }
	float			getAcceleration() const { return mAcceleration; }
	float			getRotationAcceleration() const { return mRotateAccel; }
	const DIM_T&	getDimension() { return mDimensions; }
	
protected:
	VEC_T		mPosition, mVelocity;
	ROT_T		mAngle, mRotationVelocity;
	DIM_T		mDimensions;
	float		mAcceleration, mRotateAccel;
	float		mGeometry;
};

struct Blob2D : public detail::Blob<ci::vec2, float, ci::vec2> {
	Blob2D( const osc::Message &msg );
	float getArea() const { return mGeometry; }
};
struct Blob25D : public detail::Blob<ci::vec3, float, ci::vec2> {
	Blob25D( const osc::Message &msg );
	float getArea() const { return mGeometry; }
};
struct Blob3D : public detail::Blob<ci::vec3, ci::vec3, ci::vec3> {
	Blob3D( const osc::Message &msg );
	float getVolume() const { return mGeometry; }
};

struct ProfileHandlerBase  {
	virtual ~ProfileHandlerBase() = default;
	virtual void handleMessage( const osc::Message &message ) = 0;
};

template<typename T>
using ProfileFn = Listener::ProfileFn<T>;

template<typename CallbackType, typename ProfileType = CallbackType>
struct ProfileHandler : public ProfileHandlerBase {
	//! TODO: Need to figure out about "PastFrameThreshold", got rid of it.
	ProfileHandler() {}
	
	void setAddHandler( ProfileFn<CallbackType> callback );
	void setUpdateHandler( ProfileFn<CallbackType> callback );
	void setRemoveHandler( ProfileFn<CallbackType> callback );
	void handleMessage( const osc::Message &message ) override;
	void handleAdds( std::vector<int32_t> &deleteIds );
	void handleUpdates();
	void handleRemoves();
	
	std::vector<ProfileType> getCurrentActiveProfiles();
	
	std::string								mCurrentSource;
	std::vector<int32_t>					mAdded, mUpdated;
	std::vector<ProfileType>				mSetOfCurrentTouches,
	mNextFrameTouches,
	mRemovedTouches;
	std::set<int32_t>						mCurrentAliveIds;
	std::map<std::string, int32_t>			mSourceFrameNums;
	// containers for changes which will be propagated upon receipt of 'fseq'
	ProfileFn<CallbackType>		mAddCallback, mUpdateCallback, mRemoveCallback;
	std::mutex					mAddMutex, mUpdateMutex, mRemoveMutex;
};

template<>
struct ProfileHandler<ci::app::TouchEvent, Cursor2D> : public ProfileHandlerBase {
	//! TODO: Need to figure out about "PastFrameThreshold", got rid of it.
	ProfileHandler() {}
	
	void setAddHandler( ProfileFn<ci::app::TouchEvent> callback );
	void setUpdateHandler( ProfileFn<ci::app::TouchEvent> callback );
	void setRemoveHandler( ProfileFn<ci::app::TouchEvent> callback );
	void handleMessage( const osc::Message &message ) override;
	
	std::vector<Cursor2D> getCurrentActiveProfiles();
	
	std::string								mCurrentSource;
	std::vector<int32_t>					mAdded, mUpdated;
	std::vector<Cursor2D>					mSetOfCurrentTouches,
	mNextFrameTouches,
	mRemovedTouches;
	std::set<int32_t>						mCurrentAliveIds;
	std::map<std::string, int32_t>			mSourceFrameNums;
	// containers for changes which will be propagated upon receipt of 'fseq'
	ProfileFn<ci::app::TouchEvent>			mAddCallback, mUpdateCallback, mRemoveCallback;
	std::mutex								mAddMutex, mUpdateMutex, mRemoveMutex;
};

// function_traits implemented by https://github.com/kennytm/utils/blob/master/traits.hpp	
template <typename T>
struct function_traits
: public function_traits<decltype(&T::operator())>
{};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(Args...)>
{
	/**
	 .. type:: type result_type
	 
	 The type returned by calling an instance of the function object type *F*.
	 */
	typedef ReturnType result_type;
	
	/**
	 .. type:: type function_type
	 
	 The function type (``R(T...)``).
	 */
	typedef ReturnType function_type(Args...);
	
	/**
	 .. data:: static const size_t arity
	 
	 Number of arguments the function object will take.
	 */
	enum { arity = sizeof...(Args) };
	
	/**
	 .. type:: type arg<n>::type
	 
	 The type of the *n*-th argument.
	 */
	template <size_t i>
	struct arg
	{
		typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
	};
};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(*)(Args...)>
: public function_traits<ReturnType(Args...)>
{};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...)>
: public function_traits<ReturnType(Args...)>
{
	typedef ClassType& owner_type;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
: public function_traits<ReturnType(Args...)>
{
	typedef const ClassType& owner_type;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) volatile>
: public function_traits<ReturnType(Args...)>
{
	typedef volatile ClassType& owner_type;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const volatile>
: public function_traits<ReturnType(Args...)>
{
	typedef const volatile ClassType& owner_type;
};

template <typename FunctionType>
struct function_traits<std::function<FunctionType>>
: public function_traits<FunctionType>
{};
	
} // namespace detail

template<typename CallbackFn>
void Listener::setProfileAddedCallback( CallbackFn callback )
{
	typedef typename detail::function_traits<decltype(callback)>::template arg<0>::type traits;
	typedef typename std::decay<traits>::type TuioType;
	
	static_assert(std::is_same<tuio::Cursor2D, TuioType>::value ||
				  std::is_same<tuio::Cursor25D, TuioType>::value ||
				  std::is_same<tuio::Cursor3D, TuioType>::value ||
				  std::is_same<tuio::Object2D, TuioType>::value ||
				  std::is_same<tuio::Object25D, TuioType>::value ||
				  std::is_same<tuio::Object3D, TuioType>::value ||
				  std::is_same<tuio::Blob2D, TuioType>::value ||
				  std::is_same<tuio::Blob25D, TuioType>::value ||
				  std::is_same<tuio::Blob3D, TuioType>::value ||
				  std::is_same<ci::app::TouchEvent, TuioType>::value, "Not a supported Callback");
	static_assert(detail::function_traits<decltype(callback)>::arity == 1, "");
	
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<detail::ProfileHandler<TuioType>*>(found->second.get());
		profile->setAddHandler( callback );
	}
	else {
		// TODO: Add listener to osc here
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TuioType>>( new detail::ProfileHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::ProfileHandler<TuioType>*>(inserted.first->second.get());
		created->setAddHandler( callback );
		mListener->setListener( address, std::bind( &detail::ProfileHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}


template<typename CallbackFn>
void Listener::setProfileUpdatedCallback( CallbackFn callback )
{
	typedef typename detail::function_traits<decltype(callback)>::template arg<0>::type traits;
	typedef typename std::decay<traits>::type TuioType;
	
	static_assert(std::is_same<tuio::Cursor2D, TuioType>::value ||
				  std::is_same<tuio::Cursor25D, TuioType>::value ||
				  std::is_same<tuio::Cursor3D, TuioType>::value ||
				  std::is_same<tuio::Object2D, TuioType>::value ||
				  std::is_same<tuio::Object25D, TuioType>::value ||
				  std::is_same<tuio::Object3D, TuioType>::value ||
				  std::is_same<tuio::Blob2D, TuioType>::value ||
				  std::is_same<tuio::Blob25D, TuioType>::value ||
				  std::is_same<tuio::Blob3D, TuioType>::value ||
				  std::is_same<ci::app::TouchEvent, TuioType>::value, "Not a supported Callback");
	static_assert(detail::function_traits<decltype(callback)>::arity == 1, "");
	
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<detail::ProfileHandler<TuioType>*>(found->second.get());
		profile->setUpdateHandler( callback );
	}
	else {
		// TODO: Add listener to osc here
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TuioType>>( new detail::ProfileHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::ProfileHandler<TuioType>*>(inserted.first->second.get());
		created->setUpdateHandler( callback );
		mListener->setListener( address, std::bind( &detail::ProfileHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename CallbackFn>
void Listener::setProfileRemovedCallback( CallbackFn callback )
{
	typedef typename detail::function_traits<decltype(callback)>::template arg<0>::type traits;
	typedef typename std::decay<traits>::type TuioType;
	
	static_assert(std::is_same<tuio::Cursor2D, TuioType>::value ||
				  std::is_same<tuio::Cursor25D, TuioType>::value ||
				  std::is_same<tuio::Cursor3D, TuioType>::value ||
				  std::is_same<tuio::Object2D, TuioType>::value ||
				  std::is_same<tuio::Object25D, TuioType>::value ||
				  std::is_same<tuio::Object3D, TuioType>::value ||
				  std::is_same<tuio::Blob2D, TuioType>::value ||
				  std::is_same<tuio::Blob25D, TuioType>::value ||
				  std::is_same<tuio::Blob3D, TuioType>::value ||
				  std::is_same<ci::app::TouchEvent, TuioType>::value, "Not a supported Callback");
	static_assert(detail::function_traits<decltype(callback)>::arity == 1, "");
	
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<detail::ProfileHandler<TuioType>*>(found->second.get());
		profile->setRemoveHandler( callback );
		mListener->setListener( address, std::bind( &detail::ProfileHandlerBase::handleMessage,
												   profile.get(), std::placeholders::_1 ) );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TuioType>>( new detail::ProfileHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::ProfileHandler<TuioType>*>(inserted.first->second.get());
		created->setRemoveHandler( callback );
		mListener->setListener( address, std::bind( &detail::ProfileHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename T>
const char* Listener::getOscAddressFromType()
{
	if( std::is_same<T, Cursor2D>::value ) return "/tuio/2Dcur";
	else if( std::is_same<T, Cursor25D>::value ) return "/tuio/25Dcur";
	else if( std::is_same<T, Cursor3D>::value ) return "/tuio/3Dcur";
	else if( std::is_same<T, Object2D>::value ) return "/tuio/2Dobj";
	else if( std::is_same<T, Object25D>::value ) return "/tuio/25Dobj";
	else if( std::is_same<T, Object3D>::value ) return "/tuio/3Dobj";
	else if( std::is_same<T, Blob2D>::value ) return "/tuio/2Dblb";
	else if( std::is_same<T, Blob25D>::value ) return "/tuio/25Dblb";
	else if( std::is_same<T, Blob3D>::value ) return "/tuio/3Dblb";
	// TODO: decide if this is good. Probably not but let's see
	else if( std::is_same<T, cinder::app::TouchEvent>::value ) return "/tuio/2Dcur";
	else return "Unknown Target";
}
	
namespace detail {
	
template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::setAddHandler( ProfileFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}

template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::setUpdateHandler( ProfileFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}

template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::setRemoveHandler( ProfileFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}

template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto sessionId = message[1].int32();
		auto it = find_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
						  [sessionId]( const ProfileType &profile){
							  return sessionId == profile.getSessionId();
						  });
		if( it == mSetOfCurrentTouches.end() ) {
			ProfileType profile( message );
			
			auto insert = lower_bound( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ), profile.getSessionId(),
									  []( const ProfileType &lhs, int32_t value ){
										  return lhs.getSessionId() < value;
									  });
			mSetOfCurrentTouches.insert( insert, std::move( message ) );
			mSetOfCurrentTouches.back().setSource( mCurrentSource );
			mAdded.push_back( sessionId );
		}
		else {
			*it = std::move( ProfileType( message ) );
			it->setSource( mCurrentSource );
			mUpdated.push_back( sessionId );
		}
	}
	else if( messageType == "alive" ) {
		std::vector<int32_t> aliveIds( message.getNumArgs() - 1 );
		int i = 1;
		for( auto & aliveId : aliveIds ) {
			aliveId = message[i++].int32();
		}
		std::sort( aliveIds.begin(), aliveIds.end() );
		auto remove = remove_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
								[&aliveIds]( const ProfileType &profile ) {
									return binary_search( begin(aliveIds), end(aliveIds), profile.getSessionId() );
								});
		// TODO: Fix this.
//		mRemovedTouches.resize( std::distance( remove, mSetOfCurrentTouches.end() ) );
		std::move( remove, mSetOfCurrentTouches.end(), mRemovedTouches.begin() );
	}
	else if( messageType == "fseq" ) {
		auto frame = message[1].int32();
		int32_t prev_frame = mSourceFrameNums[mCurrentSource];
		int32_t delta_frame = frame - prev_frame;
		// TODO: figure out about past frame threshold updating, this was also in the if condition ( dframe < -mPastFrameThreshold )
		if( frame == -1 || delta_frame > 0 ) {
			auto begin = mSetOfCurrentTouches.cbegin();
			auto end = mSetOfCurrentTouches.cend();
			if( ! mAdded.empty() ) {
				std::lock_guard<std::mutex> lock( mAddMutex );
				if( mAddCallback ) {
					for( auto & added : mAdded ) {
						auto found = find_if( begin, end,
						[added]( const ProfileType &profile ) {
							 return added == profile.getSessionId();
						});
						if( found != end )
							mAddCallback( *found );
					}
				}
			}
			if ( ! mUpdated.empty() ) {
				std::lock_guard<std::mutex> lock( mUpdateMutex );
				if( mUpdateCallback ) {
					for( auto & updated : mUpdated ) {
						auto found = find_if( begin, end,
						[updated]( const ProfileType &profile ) {
							 return updated == profile.getSessionId();
						});
						if( found != end )
							mUpdateCallback( *found );
					}
				}
			}
			if( ! mRemovedTouches.empty() ){
				std::lock_guard<std::mutex> lock( mRemoveMutex );
				if( mRemoveCallback )
					for( auto & removed : mRemovedTouches ) {
						mRemoveCallback( removed );
					}
			}
			
			//			mPreviousFrame[source] = ( frame == -1 ) ? mPreviousFrame[source] : frame;
		}
	}
}
}
} } // namespace tuio // namespace cinder
