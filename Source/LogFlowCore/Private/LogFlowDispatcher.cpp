#include "LogFlowDispatcher.h"

FLogFlowDispatcher::FLogFlowDispatcher()
{
	
}

FLogFlowDispatcher::~FLogFlowDispatcher()
{
	Consumers.Empty();
}

void FLogFlowDispatcher::RegisterConsumer(ILogFlowConsumer* Consumer)
{
	if (Consumer == nullptr) return;
	
	if (!Consumers.Contains(Consumer)) Consumers.Add(Consumer);
}

void FLogFlowDispatcher::UnregisterConsumer(ILogFlowConsumer* Consumer)
{
	if (Consumer == nullptr) return;
	Consumers.Remove(Consumer);
}

void FLogFlowDispatcher::Dispatch(const FLogFlowEntry& Entry)
{
	EntryQueue.Enqueue(Entry);
}

void FLogFlowDispatcher::NotifyAll()
{
	if (Consumers.Num() == 0)
	{
		// Drain the queue silently if there are no consumers.
		FLogFlowEntry Discarded;
		while (EntryQueue.Dequeue(Discarded)){}
		return;
	}
	
	FLogFlowEntry Entry;
	while (EntryQueue.Dequeue(Entry))
	{
		for (ILogFlowConsumer* Consumer : Consumers)
		{
			if (Consumer != nullptr)
			{
				Consumer->OnLogFlowEntryReceived(Entry);
			}
		}
	}
}

int32 FLogFlowDispatcher::GetConsumerCount() const
{
	return Consumers.Num();
}
