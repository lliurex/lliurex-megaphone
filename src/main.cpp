
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>


#include <iostream>
#include <string>

#include <gtkmm.h>
#include <glibmm.h>

#include <libintl.h>
#include <locale.h>

#define T(String) gettext(String)

using namespace std;

/* callbacks forward declaration */
void pa_state_callback(pa_context *c, void *userdata);
void pa_module_callback(pa_context *c, const pa_module_info *i, int eol, void *userdata);
void pa_module_index_callback(pa_context *c, uint32_t idx, void *userdata);
void pa_operation_callback(pa_context *c, int success, void *userdata);


/**
 Main application class
*/
class Application
{
	private:
	
	Glib::RefPtr<Gtk::Builder> glade;
	Gtk::Window * win;
	Gtk::Switch * sw;
	Gtk::Label * lblStatus;
	Gtk::Button * btnAccept;
	Gtk::Image * imgStatus;
	
	/* pulseaudio variables */
	pa_context * pa_ctx;
	pa_glib_mainloop * pa_ml;
	pa_mainloop_api * pa_ml_api;
	
	int id_module;
	
	public:
	
	
	
	/*!
		Constructor
	*/
	Application()
	{
		string glade_path;
		
		if(Gio::File::create_for_path("interface.glade")->query_exists())
		{
			glade_path="interface.glade";
		}
		else
		{
			if(Gio::File::create_for_path("/usr/share/lliurex-megaphone/rsrc/interface.glade")->query_exists())
			{
				glade_path="/usr/share/lliurex-megaphone/rsrc/interface.glade";
			}
			else
			{
				cerr<<"Can't find glade resource"<<endl;
				std::exit(1);
			}
			
		}
		
		
		cout<<"* Using glade resource:"<<glade_path<<endl;
		
		glade=Gtk::Builder::create_from_file(glade_path);
		glade->get_widget("winMain",win);	
		glade->get_widget("btnAccept",btnAccept);
		glade->get_widget("switchMic",sw);
		glade->get_widget("lblStatus",lblStatus);
		glade->get_widget("imgStatus",imgStatus);
		
		
		win->signal_delete_event().connect(sigc::mem_fun(*this,&Application::OnWinMainClose));
		btnAccept->signal_clicked().connect(sigc::mem_fun(*this,&Application::OnBtnAcceptClick));
		sw->property_active().signal_changed().connect(sigc::mem_fun(*this,&Application::OnSwitchMicChanged));
				
		
		win->show_all();
		
		
		
		/* pulseaudio connection */
		
		GMainContext * mc = Glib::MainContext::get_default()->gobj();
		pa_ml=pa_glib_mainloop_new(mc);
		pa_ml_api = pa_glib_mainloop_get_api(pa_ml);
		pa_ctx = pa_context_new(pa_ml_api,"lliurex-megaphone");
		

		pa_context_set_state_callback(pa_ctx,pa_state_callback,this);
		pa_context_connect(pa_ctx,nullptr,PA_CONTEXT_NOFAIL ,nullptr);
	}
	
	
	/*!
		Switch changed event
	*/
	void OnSwitchMicChanged()
	{
		cout<<"switch swapped"<<endl;
		
		if(sw->get_active())
		{
			if(id_module<0)
			{
				cout<<"Loading module..."<<endl;
				sw->set_sensitive(false);
				pa_context_load_module(pa_ctx,"module-loopback","latency_msec=1",pa_module_index_callback,this);
			}
			else
			{
				cout<<"module already loaded"<<endl;
			}
		}
		else
		{
			cout<<"Unloading module..."<<endl;
			sw->set_sensitive(false);
			pa_context_unload_module(pa_ctx,id_module,pa_operation_callback,this);
		}
	
	}
	
	/*!
		Window close event
	*/
	bool OnWinMainClose(GdkEventAny* event)
	{
		//Gtk::Main::quit();
		pa_context_disconnect(pa_ctx);
		return false;
	}
	
	/*!
		Button click event
	*/
	void OnBtnAcceptClick()
	{
		//Gtk::Main::quit();
		pa_context_disconnect(pa_ctx);
	}
	
	
	/*!
		Pulseaudio connection established
	*/
	void Connected()
	{
		sw->set_sensitive(true);
		Update();
	}
	
	/*!
		Check for loaded modules and updates gui status
	*/
	void Update()
	{
		ModuleFound(-1);
		pa_context_get_module_info_list(pa_ctx,pa_module_callback,this);
	}
	
	/*!
		Sets the id of the module
		\param id module identifier, less or equal 0 means module has not been found
	*/
	void ModuleFound(int id)
	{
		if(id>0)
		{
			id_module=id;
			sw->set_active(true);
			sw->set_sensitive(true);
			imgStatus->set_sensitive(true);
			lblStatus->set_text(T("Megaphone active"));
		}
		else
		{
			id_module=-1;
			sw->set_active(false);
			sw->set_sensitive(true);
			imgStatus->set_sensitive(false);
			lblStatus->set_text(T("Megaphone inactive"));
		}
		
		cout<<"module id: "<<id_module<<endl;
	}
	
	/*!
		Invoked whenever pulse audio is already unloaded and gui is ready to quit
	*/
	void ReadyToQuit()
	{
		//pa_glib_mainloop_free(pa_ml);
		Gtk::Main::quit();
	}
	
		
};


/*!
	Status callback
*/
void pa_state_callback(pa_context *c, void *userdata)
{

	switch (pa_context_get_state(c))
	{ 
		case PA_CONTEXT_CONNECTING:
			cout<<"Connecting"<<endl;
		break;
		
		case PA_CONTEXT_AUTHORIZING:
		break;
		
		case PA_CONTEXT_SETTING_NAME:
		break;
		
		case PA_CONTEXT_READY:
			cout<<"Ready"<<endl;
			
			static_cast<Application*>(userdata)->Connected();
			
		break;
		
		case PA_CONTEXT_TERMINATED:
			cout<<"Succesfully disconnect from pulseaudio"<<endl;
			static_cast<Application*>(userdata)->ReadyToQuit();
		break;
		
		case PA_CONTEXT_FAILED:
			cout<<"Failed to connect to pulseaudio"<<endl;
		break;
	}
}


/*!
	module info callback
*/
void pa_module_callback(pa_context *c, const pa_module_info *i, int eol, void *userdata)
{

	if(eol)return;

	cout<<"module: "<<i->name<<endl;
	
	if(string(i->name)=="module-loopback")
	{
		static_cast<Application*>(userdata)->ModuleFound(i->index);
	}


}

/*!
	module loaded callback
*/
void pa_module_index_callback(pa_context *c, uint32_t idx, void *userdata)
{
	static_cast<Application*>(userdata)->ModuleFound(idx);
}

/*!
	common operation callback, used as module unload status callback
*/
void pa_operation_callback(pa_context *c, int success, void *userdata)
{
	if(success)
	{
		static_cast<Application*>(userdata)->ModuleFound(-1);
	}
	else
	{
		cerr<<"Cannot unload module"<<endl;
	}


}

/*!
	main :)
*/
int main(int argc,char * argv[])
{

	cout<<"*****************"<<endl;
	cout<<"lliurex megaphone"<<endl;
	cout<<"*****************"<<endl;
	
	
	textdomain("lliurex-megaphone");
	Gtk::Main kit(argc, argv);
	Application app;
	Gtk::Main::run();
	
	cout<<"bye"<<endl;

 	return 0;
}
