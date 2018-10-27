#pragma once

#include "DebugRenderInterface.h"

#include "Engine/TextField.h"

#include "Kernel/ServiceBase.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    class DebugRenderService
        : public ServiceBase<DebugRenderServiceInterface>
    {
    public:
        DebugRenderService();
        ~DebugRenderService() override;

    public:
        bool _initializeService() override;
        void _finalizeService() override;

    public:
        void renderDebugNode( const NodePtr & _node, const RenderContext * _context, bool _external ) override;
        
    public:
        void addDebugNodeRenderVisitor( const RenderVisitorPtr & _renderVisitor ) override;
        void removeDebugNodeRenderVisitor( const RenderVisitorPtr & _renderVisitor ) override;

    public:
        void renderDebugInfo( const RenderContext * _context ) override;

    protected:
        void toggleDebugText_();

    protected:
        typedef Vector<RenderVisitorPtr> VectorRenderVisitor;
        VectorRenderVisitor m_renderVisitors;

        TextFieldPtr m_debugText;

        uint32_t m_showDebugText;
        uint32_t m_globalKeyHandlerF9;
    };
}