//
//  Tuio.cpp
//  TUIOListener
//
//  Created by ryan bartley on 11/5/15.
//
//

#include "Tuio.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace asio;
using namespace asio::ip;

namespace cinder { namespace  tuio {
	
namespace detail {
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Profile
	
Profile::Profile( const osc::Message &msg )
: mSessionId( msg[1].int32() )
{
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Cursor

template<>
Cursor<ci::vec2>::Cursor( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt() ),
	mVelocity( msg[4].flt(), msg[5].flt() ), mAcceleration( msg[6].flt() )
{
}
	
template<>
Cursor<ci::vec3>::Cursor( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
mVelocity( msg[5].flt(), msg[6].flt(), msg[7].flt() ), mAcceleration( msg[8].flt() )
{
}
	
template<typename T>
app::TouchEvent::Touch Cursor<T>::convertToTouch() const
{
	app::TouchEvent::Touch ret( getPosition(),
								getPosition() - getVelocity(),
								getSessionId(),
								app::getElapsedSeconds(),
								nullptr );
	return ret;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Object
	
template<>
Object<ci::vec2, float>::Object( const osc::Message &msg )
: Profile( msg ), mClassId( msg[2].int32() ), mPosition( msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt() ), mVelocity( msg[6].flt(), msg[7].flt() ),
	mRotationVelocity( msg[8].flt() ), mAcceleration( msg[9].flt() ),
	mRotateAccel( msg[10].flt() )
{
}
	
template<>
Object<ci::vec3, float>::Object( const osc::Message &msg )
: Profile( msg ), mClassId( msg[2].int32() ),
	mPosition( msg[3].flt(), msg[4].flt(), msg[5].flt() ),
	mAngle( msg[6].flt() ), mVelocity( msg[7].flt(), msg[8].flt(), msg[9].flt() ),
	mRotationVelocity( msg[10].flt() ), mAcceleration( msg[11].flt() ),
	mRotateAccel( msg[12].flt() )
{
}
	
template<>
Object<ci::vec3, ci::vec3>::Object( const osc::Message &msg )
: Profile( msg ), mClassId( msg[2].int32() ),
	mPosition(	msg[3].flt(), msg[4].flt(), msg[5].flt() ),
	mAngle(		msg[6].flt(), msg[7].flt(), msg[8].flt() ),
	mVelocity(	msg[9].flt(), msg[10].flt(), msg[11].flt() ),
	mRotationVelocity( msg[12].flt(), msg[13].flt(), msg[14].flt() ),
	mAcceleration( msg[15].flt() ), mRotateAccel( msg[16].flt() )
{
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob
	
template<>
Blob<ci::vec2, float, ci::vec2>::Blob( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt() ), mAngle( msg[4].flt() ),
	mDimensions( msg[5].flt(), msg[6].flt() ), mGeometry( msg[7].flt() ),
	mVelocity( msg[8].flt(), msg[9].flt() ), mRotationVelocity( msg[10].flt() ),
	mAcceleration( msg[11].flt() ), mRotateAccel( msg[12].flt() )
{
}
	
template<>
Blob<ci::vec3, float, ci::vec2>::Blob( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt() ), mDimensions( msg[6].flt(), msg[7].flt() ),
	mGeometry( msg[8].flt() ), mVelocity( msg[9].flt(), msg[10].flt(), msg[11].flt() ),
	mRotationVelocity( msg[12].flt() ), mAcceleration( msg[13].flt() ),
	mRotateAccel( msg[14].flt() )
{
}
	
template<>
Blob<ci::vec3, ci::vec3, ci::vec3>::Blob( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt(), msg[6].flt(), msg[7].flt() ),
	mDimensions( msg[8].flt(), msg[9].flt(), msg[10].flt() ),
	mGeometry( msg[11].flt() ), mVelocity( msg[12].flt(), msg[13].flt(), msg[14].flt() ),
	mRotationVelocity( msg[15].flt(), msg[16].flt(), msg[17].flt() ),
	mAcceleration( msg[18].flt() ), mRotateAccel( msg[19].flt() )
{
}
	
} // namespace detail
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob2D
	
Blob2D::Blob2D( const osc::Message &msg )
: Blob( msg )
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob25D
	
Blob25D::Blob25D( const osc::Message &msg )
: Blob( msg )
{
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob3D

Blob3D::Blob3D( const osc::Message &msg )
: Blob( msg )
{
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Listener
	
Listener::Listener( const app::WindowRef &window,  uint16_t localPort, const asio::ip::udp &protocol, asio::io_service &io )
: mListener( new osc::ReceiverUdp( localPort, protocol, io ) ), mWindow( window )
{
	
}
	
Listener::Listener( const osc::ReceiverBase *ptr )
{
	
}

void Listener::bind()
{
	mListener->bind();
}
	
void Listener::listen()
{
	mListener->listen();
}

void Listener::close()
{
	mListener->close();
}

template<typename ProfileType>
vector<ProfileType> Listener::getActiveProfiles() const
{
//	double currentTime = app::getElapsedSeconds();
	vector<app::TouchEvent::Touch> result;
//	if ( source == "" ) {
//		// Get cursors from all sources
//		auto & sources = getSources();
//		int sourcenum = 0;
//		for( auto & source :sources ) {
//			vector<Cursor> cursors = mHandlerCursor->getInstancesAsVector( source );
//			for( auto & inst : cursors ) {
//				result.push_back( inst.getTouch( currentTime, app::getWindowSize() ) );
//			}
//			++sourcenum;
//		}
//	} else {
//		// Get cursors from one source
//		auto cursors = mHandlerCursor->getInstancesAsVector(source);
//		for( auto & inst : cursors ) {
//			result.push_back( inst.getTouch( currentTime, app::getWindowSize() ) );
//		}
//	}
//	
	return result;	
}


	
//template<>
//void Listener::setProfileAddedCallback( ProfileFn<TouchEvent> callback )
//{
//	auto address = getOscAddressFromType<TouchEvent>();
//	auto found = mHandlers.find( address );
//	if( found != mHandlers.end() ) {
//		auto profile = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(found->second.get());
//		profile->setAddHandler( callback );
//	}
//	else {
//		// TODO: Add listener to osc here
//		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TouchEvent, Cursor2D>>( new detail::ProfileHandler<TouchEvent, Cursor2D>() ) );
//		auto created = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
//		created->setAddHandler( callback );
//	}
//}
//
//	
//template<>
//void Listener::setProfileUpdatedCallback( ProfileFn<TouchEvent> callback )
//{
//	auto address = getOscAddressFromType<TouchEvent>();
//	auto found = mHandlers.find( address );
//	if( found != mHandlers.end() ) {
//		auto profile = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(found->second.get());
//		profile->setAddHandler( callback );
//	}
//	else {
//		// TODO: Add listener to osc here
//		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TouchEvent, Cursor2D>>( new detail::ProfileHandler<TouchEvent, Cursor2D>() ) );
//		auto created = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
//		created->setUpdateHandler( callback );
//	}
//}
//
//template<>
//void Listener::setProfileRemovedCallback( ProfileFn<TouchEvent> callback )
//{
//	auto address = getOscAddressFromType<TouchEvent>();
//	auto found = mHandlers.find( address );
//	if( found != mHandlers.end() ) {
//		auto profile = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(found->second.get());
//		profile->setAddHandler( callback );
//	}
//	else {
//		// TODO: Add listener to osc here
//		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TouchEvent, Cursor2D>>( new detail::ProfileHandler<TouchEvent, Cursor2D>() ) );
//		auto created = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
//		created->setRemoveHandler( callback );
//	}
//}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Detail
	
namespace detail {
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// ProfileHandler templated
	

	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// ProfileHandler specialized (callback - TouchEvent, profile - Cursor2D)
	
void ProfileHandler<TouchEvent, Cursor2D>::setAddHandler( ProfileFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}
	
void ProfileHandler<TouchEvent, Cursor2D>::setUpdateHandler( ProfileFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}

void ProfileHandler<TouchEvent, Cursor2D>::setRemoveHandler( ProfileFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}
	
void ProfileHandler<TouchEvent, Cursor2D>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto sessionId = message[1].int32();
		auto it = find_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
		[sessionId]( const Cursor2D &profile){
			return sessionId == profile.getSessionId();
		});
		if( it == mSetOfCurrentTouches.end() ) {
			Cursor2D profile( message );
			// TODO: this lambda is wrong.
			auto insert = lower_bound( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ), profile.getSessionId(),
			[]( const Cursor2D &lhs, const int val ){
				return lhs.getSessionId() < val;
			});
			mSetOfCurrentTouches.insert( insert, std::move( message ) );
			mSetOfCurrentTouches.back().setSource( mCurrentSource );
			mAdded.push_back( sessionId );
		}
		else {
			*it = std::move( Cursor2D( message ) );
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
		[&aliveIds]( const Cursor2D &profile ) {
			return binary_search( begin(aliveIds), end(aliveIds), profile.getSessionId() );
		});
		std::move( remove, mSetOfCurrentTouches.end(),
				  std::inserter( mRemovedTouches, mRemovedTouches.begin() ) );
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
					std::vector<TouchEvent::Touch> mAddTouches;
					for( auto & added : mAdded ) {
						auto found = find_if( begin, end,
						[added]( const Cursor2D &profile ){
							 return added == profile.getSessionId();
						});
						if( found != end )
							mAddTouches.emplace_back( found->convertToTouch() );
					}
					mAddCallback( TouchEvent( ci::app::getWindow(), mAddTouches ) );
				}
			}
			if ( ! mUpdated.empty() ) {
				std::lock_guard<std::mutex> lock( mUpdateMutex );
				if( mUpdateCallback ) {
					std::vector<TouchEvent::Touch> mUpdateTouches;
					for( auto & updated : mUpdated ) {
						auto found = find_if( begin, end,
						[updated]( const Cursor2D &profile ){
							 return updated == profile.getSessionId();
						});
						if( found != end )
							mUpdateTouches.emplace_back( found->convertToTouch() );
					}
					mUpdateCallback( TouchEvent( ci::app::getWindow(), mUpdateTouches ) );
				}
			}
			if( ! mRemovedTouches.empty() ){
				std::lock_guard<std::mutex> lock( mRemoveMutex );
				if( mRemoveCallback ) {
					std::vector<TouchEvent::Touch> mRemoveTouches;
					for( auto & removed : mRemovedTouches ) {
						mRemoveTouches.emplace_back(  removed.convertToTouch() );
					}
					mRemoveCallback( TouchEvent( ci::app::getWindow(), mRemoveTouches ) );
				}
			}
			
			//mPreviousFrame[source] = ( frame == -1 ) ? mPreviousFrame[source] : frame;
		}
	}
}
		
}}} // namespace detail // namespace tuio // namespace cinder