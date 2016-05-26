#include "index/document_index_view.h"

#include <memory>
#include <string>
#include <utility>

#include "data/document.h"
#include "data/document_id.h"
#include "data/document_update_request.h"
#include "index/document_index_manager.h"

namespace redgiant {

DocumentIndexView::DocumentIndexView(DocumentIndexManager* index,
    JobExecutor<DocumentUpdateRequest>* pipeline)
: index_(index), update_pipeline_(pipeline) {
}

void DocumentIndexView::update_document(const std::string& uuid, time_t expire_time, std::shared_ptr<Document> doc) {
  index_->update(std::move(doc), expire_time);
}

void DocumentIndexView::update_document_async(const std::string& uuid, time_t expire_time, std::shared_ptr<Document> doc) {
  update_pipeline_->schedule(std::make_shared<DocumentUpdateRequest>(std::move(doc), expire_time));
}

void DocumentIndexView::remove_document(const std::string& uuid) {
  index_->remove(DocumentId(uuid));
}


} /* namespace redgiant */
